#include "PluginManager.h"
#include "GameController.h"
#include "Game.h"

#include <ConsoleApp.h>
#include "ConsolePlayer.h"

#include <iostream>
#include <iomanip>
#include <stdlib.h>

using namespace qcore::literals;

typedef const qcli::ConsoleApp::CliArgs& qarg;

qcore::GameController GC;

enum TEXT_ASCII_COLOR
{
    OFF,
    BLACK,
    RED,
    GREEN,
    YELLOW,
    BLUE,
    MAGENTA,
    CYAN,
    WHITE,
};

inline std::ostream& operator<<(std::ostream &os, const TEXT_ASCII_COLOR color)
{
    const std::map<TEXT_ASCII_COLOR, std::string> ascii_color_map = {
        {TEXT_ASCII_COLOR::OFF, "\033[0m"},
        {TEXT_ASCII_COLOR::BLACK, "\x1b[30m"},
        {TEXT_ASCII_COLOR::RED, "\x1b[31m"},
        {TEXT_ASCII_COLOR::GREEN, "\x1b[32m"},
        {TEXT_ASCII_COLOR::YELLOW, "\x1b[33m"},
        {TEXT_ASCII_COLOR::BLUE, "\x1b[34m"},
        {TEXT_ASCII_COLOR::MAGENTA, "\x1b[35m"},
        {TEXT_ASCII_COLOR::CYAN, "\x1b[36m"},
        {TEXT_ASCII_COLOR::WHITE, "\x1b[37m"}
    };

    os << ascii_color_map.find(color)->second;

    return os;
}

void PrintPlayerInfo(const std::string info, bool highlight)
{
   std::cout << (highlight ? TEXT_ASCII_COLOR::RED : TEXT_ASCII_COLOR::OFF) << std::setw(25) << info << TEXT_ASCII_COLOR::OFF;
}

std::string DurationToString(uint32_t ms)
{
   std::stringstream ss;
   ss << std::setprecision(4) << ms / 1000.0 << " sec";
   return ss.str();
}

void PrintAsciiGameBoard()
{
#ifdef WIN32
    const std::string TABLE_VERTICAL_BORDER = std::string(1, static_cast<char>(186));
    const std::string TABLE_HORIZONTAL_BORDER = std::string(1, static_cast<char>(205));
    const std::string TABLE_BOTTOM_LEFT_BORDER = std::string(1, static_cast<char>(200));
    const std::string TABLE_BOTTOM_RIGHT_BORDER = std::string(1, static_cast<char>(188));
    const std::string TABLE_TOP_LEFT_BORDER = std::string(1, static_cast<char>(201));
    const std::string TABLE_TOP_RIGHT_BORDER = std::string(1, static_cast<char>(187));
    const std::string QUORIDOR_HORIZONTAL_WALL = std::string(3, static_cast<char>(196));
    const std::string QUORIDOR_VERTICAL_WALL = std::string(1, static_cast<char>(179));
#else 
    const std::string TABLE_VERTICAL_BORDER = std::string("\u2551");
    const std::string TABLE_HORIZONTAL_BORDER = std::string("\u2550");
    const std::string TABLE_BOTTOM_LEFT_BORDER = std::string("\u255A");
    const std::string TABLE_BOTTOM_RIGHT_BORDER = std::string("\u255D");
    const std::string TABLE_TOP_LEFT_BORDER = std::string("\u2554");
    const std::string TABLE_TOP_RIGHT_BORDER = std::string("\u2557");
    const std::string QUORIDOR_HORIZONTAL_WALL = std::string("\u2500\u2500\u2500");
    const std::string QUORIDOR_VERTICAL_WALL = std::string("\u2502");
#endif

    const std::string TABLE_TOP_MARGIN = "\n     ";
    const std::string TABLE_BOTTOM_MARGIN = "\n";
    const std::string TABLE_LEFT_MARGIN = "   ";
    const std::string TABLE_RIGHT_MARGIN = " ";
    const int TABLE_LEFT_INDEX_WIDTH = 2;
    const std::string TABLE_LEFT_PADDING = " ";
    const std::string TABLE_RIGHT_PADDING = " ";

   qcore::BoardMap map, coloredMap;
   GC.getBoardState()->createBoardMap(map, 0);
   auto lastAction = GC.getBoardState()->getLastAction();

   if (lastAction.isValid())
   {
      if (lastAction.actionType == qcore::ActionType::Move)
      {
         coloredMap(lastAction.playerPosition * 2) = 1;
      }
      else
      {
         if (lastAction.wallState.orientation == qcore::Orientation::Vertical)
         {
            auto p = lastAction.wallState.position * 2 - 1_y;
            coloredMap(p) = coloredMap(p + 1_x) = coloredMap(p + 2_x) = 1;
         }
         else
         {
            auto p = lastAction.wallState.position * 2 - 1_x;
            coloredMap(p) = coloredMap(p + 1_y) = coloredMap(p + 2_y) = 1;
         }
      }
   }

   system("clear");

   std::cout << TABLE_TOP_MARGIN;

   for (int i = 0; i < qcore::BOARD_SIZE; ++i)
   {
      std::cout << std::setfill(' ') << std::setw(6) << i;
   }

   std::cout <<  "\n";
   std::cout << TABLE_LEFT_MARGIN << "0 " << TABLE_TOP_LEFT_BORDER;

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << TABLE_HORIZONTAL_BORDER;
   }

   std::cout << TABLE_TOP_RIGHT_BORDER << TABLE_RIGHT_MARGIN << "\n";

   for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
   {
      std::cout << TABLE_LEFT_MARGIN;

      if (i & 1)
      {
         std::cout << std::setw(TABLE_LEFT_INDEX_WIDTH) << std::left << (i / 2) + 1 << TABLE_VERTICAL_BORDER;
      }
      else
      {
         std::cout << std::string(TABLE_LEFT_INDEX_WIDTH, ' ') << TABLE_VERTICAL_BORDER;
      }

      std::cout << TABLE_LEFT_PADDING;

      for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
      {
         if (coloredMap(i, j))
         {
            std::cout << TEXT_ASCII_COLOR::RED;
         }

         switch (map(i, j))
         {
            case 0:
               std::cout << "   ";
               break;
            case qcore::BoardMap::VertivalWall:
               std::cout << " " << QUORIDOR_VERTICAL_WALL << " ";
               break;
            case qcore::BoardMap::HorizontalWall:
               std::cout << QUORIDOR_HORIZONTAL_WALL;
               break;
            case qcore::BoardMap::Pawn0:
               std::cout << " 0 ";
               break;
            case qcore::BoardMap::Pawn1:
               std::cout << " 1 ";
               break;
            case qcore::BoardMap::Pawn2:
               std::cout << " 2 ";
               break;
            case qcore::BoardMap::Pawn3:
               std::cout << " 3 ";
               break;
            default:
               std::cout << " " << map(i, j) << " ";
               break;
         }

         if (coloredMap(i, j))
         {
            std::cout << TEXT_ASCII_COLOR::OFF;
         }
      }

      std::cout << TABLE_RIGHT_PADDING << TABLE_VERTICAL_BORDER  << TABLE_RIGHT_MARGIN << "\n";
   }

   std::cout << TABLE_LEFT_MARGIN << std::string(TABLE_LEFT_INDEX_WIDTH, ' ') << TABLE_BOTTOM_LEFT_BORDER;

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << TABLE_HORIZONTAL_BORDER;
   }

   std::cout << TABLE_BOTTOM_RIGHT_BORDER << TABLE_RIGHT_MARGIN << "\n" << TABLE_BOTTOM_MARGIN;

   qcore::PlayerId pId = GC.getGame()->getCurrentPlayer() ^ 1;

   // Print players state
   std::cout << "                    ";
   PrintPlayerInfo("P0: " + GC.getPlayer(0)->getName(), pId == 0);
   PrintPlayerInfo("P1: " + GC.getPlayer(1)->getName(), pId == 1);

   std::cout << "\n Walls:             ";
   PrintPlayerInfo(std::to_string((int)GC.getPlayer(0)->getWallsLeft()), pId == 0);
   PrintPlayerInfo(std::to_string((int)GC.getPlayer(1)->getWallsLeft()), pId == 1);

   std::cout << "\n Move duration:     ";
   PrintPlayerInfo(DurationToString(GC.getPlayer(0)->getLastMoveDuration()), pId == 0);
   PrintPlayerInfo(DurationToString(GC.getPlayer(1)->getLastMoveDuration()), pId == 1);
   std::cout << "\n\n";

   if (GC.getBoardState()->isFinished())
   {
      std::cout << TEXT_ASCII_COLOR::GREEN << "                        "
         << GC.getPlayer(GC.getBoardState()->getWinner())->getName() << " won!"
         << TEXT_ASCII_COLOR::OFF << "\n";
   }
}

void RunCommand_Move(std::ostream& out, qarg args)
{
   std::string dirStr = args.getValue("<direction>");
   qcore::Direction direction = qcore::Direction::Up;

   if (dirStr == "up" or dirStr == "u")
   {
      direction = qcore::Direction::Up;
   }
   else if(dirStr == "down" or dirStr == "d")
   {
      direction = qcore::Direction::Down;
   }
   else if(dirStr == "left" or dirStr == "l")
   {
      direction = qcore::Direction::Left;
   }
   else if(dirStr == "right" or dirStr == "r")
   {
      direction = qcore::Direction::Right;
   }
   else
   {
      out << "Invalid direction '" << dirStr << "'\n";
      return;
   }

   GC.moveCurrentPlayer(direction);
}

void RunCommand_MoveTo(std::ostream&, qarg args)
{
   GC.moveCurrentPlayer(qcore::Position(std::stoi(args.getValue("<x>")), std::stoi(args.getValue("<y>"))));
}

void RunCommand_Wall(std::ostream& out, qarg args)
{
   uint8_t x = std::stoi(args.getValue("<x>"));
   uint8_t y = std::stoi(args.getValue("<y>"));
   std::string orStr = args.getValue("<orientation>");
   qcore::Orientation orientation;

   if (orStr == "v")
   {
      orientation = qcore::Orientation::Vertical;
   }
   else if(orStr == "h")
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

void RunCommand_Reset(std::ostream&, qarg args)
{
   GC.initLocalGame(args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2);
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);
}

void RunCommand_ServerDiscovery(std::ostream& out, qarg)
{
   auto endpoints = GC.discoverRemoteGames();

   for (auto& e : endpoints)
   {
        out << "   " << e.ip << ": " << e.serverName << "\n";
   }
}

void RunCommand_ServerStart(std::ostream&, qarg args)
{
   GC.startServer(args.getValue("<server-name>"), args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2);
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);
}

void RunCommand_JoinServer(std::ostream&, qarg args)
{
   GC.connectToRemoteGame(args.getValue("<server-ip>"));
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);
}

int main(int argc, char *argv[])
{
   qcore::PluginManager::RegisterPlugin<qcli::ConsolePlayer>("qcli::ConsolePlayer");

   // Initialize a local game with 2 players
   GC.initLocalGame(2);

   // Print board at every change
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);

   // Setup console application menu
   qcli::ConsoleApp app(std::cout);

   app.addCommand([](std::ostream&, qarg){ PrintAsciiGameBoard(); }, "board", "Game Setup")
      .setSummary("Prints the current state of the board.");

   app.addCommand([](std::ostream& out, qarg){ for (auto& p : qcore::PluginManager::GetPluginList()) out << "   " << p << "\n"; }, "plugins", "Game Setup")
      .setSummary("Lists all available plugins.");

   app.addCommand([](std::ostream&, qarg a){ GC.addPlayer(a.getValue("<plugin>"), a.getValue("<player-name>")); }, "add player <plugin> <player-name>", "Game Setup")
      .setSummary("Adds a new player to the current game.")
      .setDescription("EXAMPLE:\n   add player qplugin::DummyPlayer P1\n   add player qcli::ConsolePlayer P2")
      .registerParameterAutocompleteCb("<plugin>", [](const std::string&)->std::list<std::string> {return qcore::PluginManager::GetPluginList();})
      .registerParameterAutocompleteCb(
          "<player-name>", 
          [](const std::string& cmd)->std::list<std::string> 
          {
              if (not cmd.empty()) return { cmd };
              static int c = 0; 
              c++; 
              return { std::to_string(c) };
          });

   app.addCommand([](std::ostream&, qarg a){ PrintAsciiGameBoard(); GC.start(a.isSet("-one")); }, "start -one", "Game Setup")
      .setSummary("Starts the game.");

   app.addCommand(RunCommand_Reset, "reset -p <players>", "Game Setup")
      .setSummary("Resets the current game.");

   app.addCommand(RunCommand_ServerStart, "server start <server-name> -p <players>", "Remote Game Setup")
      .setSummary("Starts a quoridor game server.");

   app.addCommand(RunCommand_JoinServer, "join <server-ip>", "Remote Game Setup")
      .setSummary("Connects to a remote game server.");

   app.addCommand(RunCommand_ServerDiscovery, "server discovery", "Remote Game Setup")
      .setSummary("Starts a server discovery and lists all found servers.");

   app.addCommand(RunCommand_Move, "move <direction>", "Player Actions")
      .setSummary("Moves the current player in the specified direction (up, down, left, right)")
      .registerParameterAutocompleteCb(
          "<direction>", 
          [](const std::string& cmd)->std::list<std::string> 
          {
              std::list<std::string> candidates = {"up", "down", "left", "right"};

              if (cmd.empty()) return candidates;

              candidates.remove_if(
                  [&cmd](const std::string& e)
                  {
                      for (size_t i = 0; i < cmd.size() && i < e.size(); i++)
                          if (toupper(cmd[i]) != toupper(e[i]))
                              return true;
                      return false;
                  });
              return candidates;
          });

   app.addCommand(RunCommand_MoveTo, "moveto <x> <y>", "Player Actions")
      .setSummary("Moves the current player to the specified position (x, y)");

   app.addCommand(RunCommand_Wall, "wall <x> <y> <orientation>", "Player Actions")
      .setSummary("Places a wall at position (x, y) with the specified orientation (v, h) ");

   app.addCommand([&app](std::ostream &, qarg){ app.printHelp(); }, "help", "General")
      .setSummary("Prints this menu");

   app.addCommand([](std::ostream&, qarg){system("clear");}, "clear", "General")
      .setSummary("Clear screen");

   // Run application
   return app.runCli(argc, argv);
}
