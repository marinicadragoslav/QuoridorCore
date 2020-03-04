#include "PluginManager.h"
#include "GameController.h"

#include "ConsoleApp.h"
#include "ConsolePlayer.h"

#include <iostream>
#include <iomanip>

using namespace qcore::literals;

typedef const qcli::ConsoleApp::CliArgs& qarg;

qcore::GameController GC;

void PrintAsciiGameBoard()
{
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

   std::cout << "\n";

   for (int i = 0; i < qcore::BOARD_SIZE; ++i)
   {
      std::cout << std::setfill(' ') << std::setw(6) << i;
   }

   std::cout << "\n   0 \u2554";

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << "\u2550";
   }

   std::cout << "\u2557\n";

   for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
   {
      if (i & 1)
      {
         std::cout << std::setfill(' ') << std::setw(4) << (i / 2) + 1 << " \u2551 ";
      }
      else
      {
         std::cout << "     \u2551 ";
      }

      for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
      {
         if (coloredMap(i, j))
         {
            std::cout << "\x1B[31m";
         }

         switch (map(i, j))
         {
            case 0:
               std::cout << "   ";
               break;
            case qcore::BoardMap::VertivalWall:
               std::cout << " \u2502 ";
               break;
            case qcore::BoardMap::HorizontalWall:
               std::cout << "\u2500\u2500\u2500";
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
            std::cout << "\033[0m";
         }
      }

      std::cout << " \u2551\n";
   }

   std::cout << "     \u255A";

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << "\u2550";
   }

   std::cout << "\u255D\n\n";
}

void RunCommand_Move(qarg args)
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
      std::cout << "Invalid direction '" << dirStr << "'\n";
      return;
   }

   GC.moveCurrentPlayer(direction);
}

void RunCommand_MoveTo(qarg args)
{
   GC.moveCurrentPlayer(qcore::Position(std::stoi(args.getValue("<x>")), std::stoi(args.getValue("<y>"))));
}

void RunCommand_Wall(qarg args)
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
      std::cout << "Invalid orientation '" << orStr << "'\n";
      return;
   }

   GC.placeWallForCurrentPlayer(qcore::Position(x, y), orientation);
}

void RunCommand_Reset(qarg args)
{
   GC.initLocalGame(args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2);
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);
}

void RunCommand_ServerDiscovery(qarg)
{
   auto endpoints = GC.discoverRemoteGames();

   for (auto& e : endpoints)
   {
      std::cout << "   " << e.ip << ": " << e.serverName << "\n";
   }
}

void RunCommand_ServerStart(qarg args)
{
   GC.startServer(args.getValue("<server-name>"), args.isSet("-p") ? std::stoi(args.getValue("<players>")) : 2);
   GC.getBoardState()->registerStateChange(PrintAsciiGameBoard);
}

void RunCommand_JoinServer(qarg args)
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
   qcli::ConsoleApp app;

   app.addCommand([](qarg){ PrintAsciiGameBoard(); }, "board", "Game Setup")
      .setSummary("Prints the current state of the board.");

   app.addCommand([](qarg){ for (auto& p : qcore::PluginManager::GetPluginList()) std::cout << "   " << p << "\n"; }, "plugins", "Game Setup")
      .setSummary("Lists all available plugins.");

   app.addCommand([](qarg a){ GC.addPlayer(a.getValue("<plugin>"), a.getValue("<player-name>")); }, "add player <plugin> <player-name>", "Game Setup")
      .setSummary("Adds a new player to the current game.")
      .setDescription("EXAMPLE:\n   add player qplugin::DummyPlayer P1\n   add player qcli::ConsolePlayer P2");

   app.addCommand([](qarg a){ PrintAsciiGameBoard(); GC.start(a.isSet("-one")); }, "start -one", "Game Setup")
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
      .setSummary("Moves the current player in the specified direction (up, down, left, right)");

   app.addCommand(RunCommand_MoveTo, "moveto <x> <y>", "Player Actions")
      .setSummary("Moves the current player to the specified position (x, y)");

   app.addCommand(RunCommand_Wall, "wall <x> <y> <orientation>", "Player Actions")
      .setSummary("Places a wall at position (x, y) with the specified orientation (v, h) ");

   // Run application
   return app.execute(argc, argv);
}
