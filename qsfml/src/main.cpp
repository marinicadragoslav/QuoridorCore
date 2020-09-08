
#include <vector>
#include <random>

#include <spdlog/spdlog.h>

#include <imgui.h>
#include <imgui-SFML.h>

#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/Window/Event.hpp>
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Sprite.hpp>

#include "GameController.h"
#include "PluginManager.h"

#include "FontLoader.h"
#include "TextFactory.h"
#include "ImGUIBoardWidget.h"
#include "ImGUIConsoleWidget.h"
#include "ImGuiTimelineWidget.h"
#include "TileMap.h"
#include "TimelineRepo.h"
#include "AnimationIncremental.h"
#include "BoardMapShow.h"
#include "ManualPlayer.h"


qcore::BoardMap::ItemType intToBoardItem(int i)
{
	switch (i)
	{
	case 0: return qcore::BoardMap::ItemType::VertivalWall;
	case 1: return qcore::BoardMap::ItemType::HorizontalWall;
	case 2: return qcore::BoardMap::ItemType::Pawn0;
	case 3: return qcore::BoardMap::ItemType::Pawn1;
	case 4: return qcore::BoardMap::ItemType::Pawn2;
	case 5: return qcore::BoardMap::ItemType::Pawn3;
	}

	return qcore::BoardMap::ItemType::Invalid;
}

static void generateMoveOnBoardMap(qcore::BoardMap& map)
{
	std::default_random_engine generator;
	generator.seed(std::random_device()());
	std::uniform_int_distribution<int> positionDistribution(0, 8 * 2 + 1);
	std::uniform_int_distribution<int> elementDistribution(0, 5);
	int i = positionDistribution(generator);
	int j = positionDistribution(generator);
	auto item = intToBoardItem(elementDistribution(generator));

	map(i, j) = item;
}


static void consoleActionsMove(qcore::GameController& GC, const qcli::ConsoleApp::CliArgs& args)
{
	std::string dirStr = args.getValue("<direction>");
	qcore::Direction direction = qcore::Direction::Up;

	if (dirStr == "up" or dirStr == "u")
	{
		direction = qcore::Direction::Up;
	}
	else if (dirStr == "down" or dirStr == "d")
	{
		direction = qcore::Direction::Down;
	}
	else if (dirStr == "left" or dirStr == "l")
	{
		direction = qcore::Direction::Left;
	}
	else if (dirStr == "right" or dirStr == "r")
	{
		direction = qcore::Direction::Right;
	}
	else
	{
		std::cout << "Invalid direction '" << dirStr << "'\n";
		return;
	}

	GC.moveCurrentPlayer(direction);
};

static std::list<std::string> consoleActionsMoveAutocomplete(const std::string& cmd)
{
	std::list<std::string> candidates = { "up", "down", "left", "right" };

	if (cmd.empty()) return candidates;

	candidates.remove_if(
		[&cmd](const std::string& e)
		{
			for (auto i = 0; i < cmd.size() && i < e.size(); i++)
				if (toupper(cmd[i]) != toupper(e[i]))
					return true;
			return false;
		});
	return candidates;
}


void consoleActionsMoveTo(qcore::GameController& GC, std::ostream& out, qcli::ConsoleApp::CliArgs& args)
{
	GC.moveCurrentPlayer(qcore::Position(std::stoi(args.getValue("<x>")), std::stoi(args.getValue("<y>"))));
}

void consoleActionsWall(qcore::GameController& GC, std::ostream& out, qcli::ConsoleApp::CliArgs& args)
{
	uint8_t x = std::stoi(args.getValue("<x>"));
	uint8_t y = std::stoi(args.getValue("<y>"));
	std::string orStr = args.getValue("<orientation>");
	qcore::Orientation orientation;

	if (orStr == "v")
	{
		orientation = qcore::Orientation::Vertical;
	}
	else if (orStr == "h")
	{
		orientation = qcore::Orientation::Horizontal;
	}
	else
	{
		out << "Invalid orientation '" << orStr << "'\n";
		return;
	}

	GC.placeWallForCurrentPlayer(qcore::Position(x, y), orientation);
}

static void ImGuiConsoleRegisterCommands(ImGUIConsoleWidget& console, qcore::GameController& GC, qcore::BoardState::StateChangeCb& stateCb, TimelineRepo &timelineRepo)
{
	console.AddCommand([&GC](std::ostream& out, auto a) { GC.start(a.isSet("-one")); }, "start -one", "Game Setup")
		.setSummary("Starts the game.");

	console.AddCommand(
		[&GC, &stateCb, &timelineRepo](std::ostream& out, auto args) 
		{ 
			GC.initLocalGame(args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2); 
			GC.getBoardState()->registerStateChange(stateCb);
			timelineRepo.dropFrom(0);
		}, "reset -p <players>", "Game Setup")
		.setSummary("Resets the current game.");

	console.AddCommand([&GC](std::ostream& out, auto args) { GC.startServer(args.getValue("<server-name>"), args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2); }, "server start <server-name> -p <players>", "Remote Game Setup")
		.setSummary("Starts a quoridor game server.");

	console.AddCommand([&GC](std::ostream& out, auto args) {  GC.connectToRemoteGame(args.getValue("<server-ip>")); }, "join <server-ip>", "Remote Game Setup")
		.setSummary("Connects to a remote game server.");

	console.AddCommand([&GC](std::ostream& out, auto args) {  auto endpoints = GC.discoverRemoteGames(); for (auto& e : endpoints) out << "   " << e.ip << ": " << e.serverName << "\n"; }, "server discovery", "Remote Game Setup")
		.setSummary("Starts a server discovery and lists all found servers.");

	console.AddCommand([](std::ostream& out, const qcli::ConsoleApp::CliArgs& ) { for (auto& p : qcore::PluginManager::GetPluginList()) out << "   " << p << "\n"; }, "plugins", "Game Setup")
		.setSummary("Lists all available plugins.");

	console.AddCommand([&GC](std::ostream& out, const qcli::ConsoleApp::CliArgs& a) { GC.addPlayer(a.getValue("<plugin>"), a.getValue("<player-name>")); }, "add player <plugin> <player-name>", "Game Setup")
		.setSummary("Adds a new player to the current game.")
		.setDescription("EXAMPLE:\n   add player qplugin::DummyPlayer P1\n   add player qcli::ConsolePlayer P2")
		.registerParameterAutocompleteCb(
			"<plugin>", 
			[](const std::string& cmd)->std::list<std::string> 
			{
				auto candidates = qcore::PluginManager::GetPluginList();
				if (cmd.empty()) return candidates;
				 
				candidates.remove_if(
					[&cmd](const std::string& e)
					{
						for (auto i = 0; i < cmd.size() && i < e.size(); i++)
							if (toupper(cmd[i]) != toupper(e[i]))
								return true;
						return false;
					});
				return candidates;
			})
		.registerParameterAutocompleteCb(
			"<player-name>",
			[](const std::string& cmd)->std::list<std::string>
			{
				if (not cmd.empty()) return { cmd };
				static int c = 0;
				c++;
				return { std::to_string(c) };
			});

	console.AddCommand([&GC](std::ostream& out, auto args) { consoleActionsMove(GC, args); }, "move <direction>", "Player Actions")
		.setSummary("Moves the current player in the specified direction (up, down, left, right)")
		.registerParameterAutocompleteCb("<direction>", [](const std::string& cmd)->std::list<std::string>{ return consoleActionsMoveAutocomplete(cmd);});

	console.AddCommand([&GC](std::ostream& out, auto args) { consoleActionsMoveTo(GC, out, args); }, "moveto <x> < y>", "Player Actions")
		.setSummary("Moves the current player to the specified position (x, y)");

	console.AddCommand([&GC](std::ostream& out, auto args) { consoleActionsWall(GC, out, args); }, "wall <x> <y> <orientation>", "Player Actions")
		.setSummary("Places a wall at position (x, y) with the specified orientation (v, h) ");
}

static void ImGuiWindowOnResize(sf::RenderWindow& window, sf::Event& event)
{
	const unsigned int WINDOW_MIN_SIZE = 900;	
	if (event.size.width < WINDOW_MIN_SIZE || event.size.height < WINDOW_MIN_SIZE)
	{
		window.setSize(sf::Vector2u(std::max(WINDOW_MIN_SIZE, event.size.width), std::max(WINDOW_MIN_SIZE, event.size.height)));
	}

	window.setView(sf::View(sf::FloatRect(0, 0, event.size.width, event.size.height)));
}

int main([[maybe_unused]] int argc, [[maybe_unused]] const char** argv)
{
   qcore::PluginManager::RegisterPlugin<ManualPlayer>("ManualPlayer");

	qcore::GameController GC;
	GC.initLocalGame(2);

	FontLoader::init();

	ImGUIConsoleWidget console;
	ImGUIBoardWidget board;
	board.setManual(true);
	ImGuiTimelineWidget timelineWidget;
	TimelineRepo timelineRepo;

	qcore::BoardState::StateChangeCb boardStateCb = [&GC, &timelineRepo]()
	{
		timelineRepo.push(TimelineEntry(GC.exportGame()));
	};
	GC.getBoardState()->registerStateChange(boardStateCb);
	{
		timelineRepo.push(TimelineEntry(GC.exportGame()));
	}

	BoardMapShowStrategy* currentShowStrategy = nullptr;
	ShowLastStrategy showLastStrategy;
	SlideShowStrategy slideShowAnimationStrategy;
	currentShowStrategy = &showLastStrategy;

	slideShowAnimationStrategy.start();

	sf::RenderWindow window(sf::VideoMode(1024, 1024), "ImGui + SFML + Quoridor = <3");
	window.setFramerateLimit(60);
	ImGui::SFML::Init(window);

	ImGuiConsoleRegisterCommands(console, GC, boardStateCb, timelineRepo);
	console.ExecCommand("help");

	std::function<void(ImGUIBoardWidget::ACTION_ID, sf::Vector2u)> boardCallback =
		[&console](ImGUIBoardWidget::ACTION_ID id, sf::Vector2u position)->void
	{
		switch (id)
		{
		case ImGUIBoardWidget::ACTION_ID::MOVE_UP:
			console.ExecCommand("move up");
			break;
		case ImGUIBoardWidget::ACTION_ID::MOVE_DOWN:
			console.ExecCommand("move down");
			break;
		case ImGUIBoardWidget::ACTION_ID::MOVE_LEFT:
			console.ExecCommand("move left");
			break;
		case ImGUIBoardWidget::ACTION_ID::MOVE_RIGHT:
			console.ExecCommand("move right");
			break;
		case ImGUIBoardWidget::ACTION_ID::ADD_HORIZONTAL_WALL:
			console.ExecCommand(std::string("wall " + std::to_string(position.x) + " " + std::to_string(position.y) + " h").c_str());
			break;
		case ImGUIBoardWidget::ACTION_ID::ADD_VERTICAL_WALL:
			console.ExecCommand(std::string("wall " + std::to_string(position.x) + " " + std::to_string(position.y) + " v").c_str());
			break;
		default:
			break;
		}
	};
	board.setCallback(boardCallback);

	std::function<void(ImGuiTimelineWidget::TIMELINE_WIDGET_ID, bool)> timelineWidgetCallback =
		[&slideShowAnimationStrategy, &currentShowStrategy, &showLastStrategy, &GC, &timelineRepo](ImGuiTimelineWidget::TIMELINE_WIDGET_ID id, bool isPressed)->void
	{
		const float TIMELINE_ANIMATION_SPEED_INCREMENT = 0.25f;
		switch (id)
		{
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::LOOP:
			slideShowAnimationStrategy.loop(isPressed);
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::LAST:
			if (isPressed)
			{
				currentShowStrategy = &showLastStrategy;
			}
			else
			{
				currentShowStrategy = &slideShowAnimationStrategy;
			}
			slideShowAnimationStrategy.stop();
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::PLAY:
			slideShowAnimationStrategy.start();
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::RESUME:
			timelineRepo.dropFrom(currentShowStrategy->getCurrentValue());
			GC.loadGame(timelineRepo[currentShowStrategy->getCurrentValue()].game);
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::STOP:
			slideShowAnimationStrategy.stop();
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::SPEED_UP:
			slideShowAnimationStrategy.setAnimationInterval(slideShowAnimationStrategy.getAnimationInterval() - TIMELINE_ANIMATION_SPEED_INCREMENT);
			break;
		case ImGuiTimelineWidget::TIMELINE_WIDGET_ID::SPEED_DOWN:
			slideShowAnimationStrategy.setAnimationInterval(slideShowAnimationStrategy.getAnimationInterval() + TIMELINE_ANIMATION_SPEED_INCREMENT);
			break;
		default:
			break;
		}
	};
	timelineWidget.setCallback(timelineWidgetCallback);

	sf::Clock deltaClock;
	while (window.isOpen()) {
		sf::Event event;
		while (window.pollEvent(event)) {
			ImGui::SFML::ProcessEvent(event);

			if (event.type == sf::Event::Closed) {
				window.close();
			}
			else if (event.type == sf::Event::Resized)
			{
				ImGuiWindowOnResize(window, event);
			}
		}

		ImGui::SFML::Update(window, deltaClock.restart());

		window.clear();

		//Draw Timeline
		int currentFrame;

		// Switch between Last frame and animation
		currentShowStrategy->setUpperLimit(timelineRepo.count() - 1);
		currentShowStrategy->Update();
		currentFrame = currentShowStrategy->getCurrentValue();

		// Let the UI overwrite currentFrame
		timelineWidget.setMaxTime(timelineRepo.count() - 1);
		timelineWidget.setCurrentTime(currentFrame);
		timelineWidget.ImGuiDrawAndUpdate();
		currentFrame = timelineWidget.getCurrentTime();

		// Sync
		currentShowStrategy->setCurrentValue(currentFrame);

		// Draw board
		board.setBoard(timelineRepo[currentFrame]);

		board.ImGuiDrawBoard();

		//Draw Console
		bool open = true;
		console.Draw("Console", &open);

		ImGui::SFML::Render(window);

		auto fpsText = TextFactory::Text(std::to_string(ImGui::GetIO().Framerate) + " fps");
		window.draw(fpsText);

		window.display();
	}

	ImGui::SFML::Shutdown();
	FontLoader::deinit();

	return 0;
}
