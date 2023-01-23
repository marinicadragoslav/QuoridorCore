#include "marinica_player.h"
#include "QcoreUtil.h"
#include "board.h"
#include "debug.h"
#include <thread>

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace qplugin
{
   /** Notes:
    *  The current plugin player will be be refered to in first person ("Me") and the other one will be "Opponent".
    *  The board will always be viewed from my perspective, with me on the bottom (last row).
    * 
    *  Player positions, as given by the game's API, are always relative (so the opponent's position always
    *  needs converting to my perspective), but a wall's position as the last action performed by the opponent 
    *  is absolute - so it needs converting when I am the second registered player (second player to move).
    */

   /** Log domain */
   const char * const DOM = "qplugin::MP";
   
   // Convert absolute wall position to relative position
   static qcore::Position AbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation)   
   {
      qcore::Position relPos;

      // flip around the board center, on both axis
      relPos.x = qcore::BOARD_SIZE - absPos.x;
      relPos.y = qcore::BOARD_SIZE - absPos.y;

      // the wall's position now refers to the wrong end of the wall, so adjust for that
      relPos.x -= (orientation == qcore::Orientation::Vertical ? 2 : 0);
      relPos.y -= (orientation == qcore::Orientation::Horizontal ? 2 : 0);

      return relPos;
   }

   MarinicaPlayer::MarinicaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** This function defines the behaviour of the player. It is called by the game when it is my turn, and it 
    *  needs to end with a call to one of the action functions: move(...), placeWall(...).
    */      
   void MarinicaPlayer::doNextMove()
   {
      static int turnCount;

      if (turnCount == 0)
      {
         InitBoard();
      }

      // get game info 
      uint8_t              myID              = getId(); // = 0 if I am the first player to move or 1 otherwise.
      uint8_t              opponentID        = (myID ? 0 : 1);
      qcore::Position      myPos             = getPosition();
      qcore::BoardStatePtr boardState        = getBoardState();
      qcore::PlayerState   opponentState     = boardState->getPlayers(opponentID).at(opponentID);
      qcore::Position      opponentPos       = (opponentState.position).rotate(2); // check Notes 
      qcore::PlayerAction  lastAct           = boardState->getLastAction();
      qcore::ActionType    lastActType       = lastAct.actionType;
      qcore::Orientation   lastActWallOrient = lastAct.wallState.orientation;
      qcore::Position      lastActWallPos    = (myID == 0 ? lastAct.wallState.position : 
                                                   AbsToRelWallPos(lastAct.wallState.position, lastActWallOrient)); // check Notes
      uint8_t              myWallsLeft       = getWallsLeft();
      uint8_t              opponentWallsLeft = opponentState.wallsLeft;

      // print game info
      LOG_INFO(DOM) << "-----------------------------------------------------";
      LOG_INFO(DOM) << "  Turn #: " << turnCount;
      LOG_INFO(DOM) << "      Me: id = " << (int)myID       << ", pos = [" << (int)myPos.x       << ", " << (int)myPos.y       << "], walls left = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "Opponent: id = " << (int)opponentID << ", pos = [" << (int)opponentPos.x << ", " << (int)opponentPos.y << "], walls left = " << (int)opponentWallsLeft;
      LOG_INFO(DOM) << "Last act: " << (lastActType == qcore::ActionType::Invalid ? "Invalid" : (lastActType == qcore::ActionType::Move ? "Move" : "Wall"));
      if (lastActType == qcore::ActionType::Wall)
      {
         LOG_INFO(DOM) << "Wall pos: [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "], orientation: " << (lastActWallOrient == qcore::Orientation::Horizontal ? "H" : " V");
      }

      UpdateMyPos({myPos.x, myPos.y});
      UpdateOpponentPos({opponentPos.x, opponentPos.y});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      PlaceVWallByMe({6, 6});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      PlaceVWallByOpponent({7, 7});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      PlaceHWallByMe({7, 6});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      UndoHWallByMe({7, 6});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      UndoVWallByOpponent({7, 7});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());

      UndoVWallByMe({6, 6});

      debug_PrintTileStructure(GetBoard());
      debug_PrintWallHStructure(GetBoard());
      debug_PrintWallVStructure(GetBoard());


      LOG_INFO(DOM) << "-----------------------------------------------------";

      turnCount++;

      // ----------------------------------------------------------------------------
      LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking..";

      // Simulate more thinking
      std::this_thread::sleep_for(1000ms);

      myPos = getPosition() * 2;
      qcore::BoardMap map;
      std::list<qcore::Position> pos;

      getBoardState()->createBoardMap(map, getId());

      for (uint8_t i = 0; i < qcore::BOARD_MAP_SIZE; i += 2)
      {
         qcore::Position p(0, i);
         map(p) = qcore::BoardMap::Invalid;
         pos.push_back(p);
      }

      auto checkPos = [&](const qcore::Position& p, qcore::Direction dir) -> bool
      {
         if (myPos == p)
         {
            move(dir);
            return true;
         }

         if(map(p) < qcore::BoardMap::HorizontalWall)
         {
            map(p) = qcore::BoardMap::Invalid;
            pos.emplace_back(p);
         }

         return false;
      };

      while (not pos.empty())
      {
         auto p = pos.front();
         pos.pop_front();

         if ((map(p + 1_x) == 0 and checkPos(p + 2_x, qcore::Direction::Up)) or
            (map(p - 1_x) == 0 and checkPos(p - 2_x, qcore::Direction::Down)) or
            (map(p + 1_y) == 0 and checkPos(p + 2_y, qcore::Direction::Left)) or
            (map(p - 1_y) == 0 and checkPos(p - 2_y, qcore::Direction::Right)))
         {
            return;
         }
      }

      LOG_WARN(DOM) << "Something went wrong. Making a random move.";
      move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }

} // namespace qplugin
