#include "marinica_player.h"
#include "QcoreUtil.h"
#include "board.h"
#include "debug.h"
#include "tests.h"
#include "min_path.h"
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
    *  is absolute - so it needs converting only when I am the second registered player (second player to move).
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
      uint8_t              myID           = getId(); // = 0 if I am the first player to move or 1 otherwise.
      uint8_t              oppID          = (myID ? 0 : 1); // opponent's ID
      qcore::Position      myPos          = getPosition();
      qcore::BoardStatePtr boardState     = getBoardState();
      qcore::PlayerState   oppState       = boardState->getPlayers(oppID).at(oppID);
      qcore::Position      oppPos         = (oppState.position).rotate(2); // check Notes above
      qcore::PlayerAction  lastAct        = boardState->getLastAction();
      qcore::ActionType    lastActType    = lastAct.actionType;
      qcore::Orientation   lastActWallOr  = lastAct.wallState.orientation;
      qcore::Position      lastActWallPos = (myID == 0 ? lastAct.wallState.position : 
                                                   AbsToRelWallPos(lastAct.wallState.position, lastActWallOr)); // check Notes above
      uint8_t              myWallsLeft    = getWallsLeft();
      uint8_t              oppWallsLeft   = oppState.wallsLeft;

      // print game info
      LOG_INFO(DOM) << "---------------------------------------------------------------";
      LOG_INFO(DOM) << "  Turn count = " << turnCount;
      LOG_INFO(DOM) << "  Me  (" << (int)myID << ") pos = [" << (int)myPos.x << ", " << (int)myPos.y << "], walls = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "  Opp (" << (int)oppID << ") pos = [" << (int)oppPos.x << ", " << (int)oppPos.y << "], walls = " << (int)oppWallsLeft;
      LOG_INFO(DOM) << "  Last act = " << (lastActType == qcore::ActionType::Invalid ? "Invalid" : 
                                                (lastActType == qcore::ActionType::Move ? "Move" : "Wall"));

      if (lastActType == qcore::ActionType::Wall && lastActWallOr == qcore::Orientation::Horizontal)
      {
         LOG_INFO(DOM) << "  Wall dir = H, wall pos (game)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
         LOG_INFO(DOM) << "                wall pos (plugin) = [" << (lastActWallPos.x - 1) << ", " << (int)lastActWallPos.y << "]";
         PlaceHorizWallByOpponent({(int8_t)(lastActWallPos.x - 1), lastActWallPos.y});
      }

      if (lastActType == qcore::ActionType::Wall && lastActWallOr == qcore::Orientation::Vertical)
      {
         LOG_INFO(DOM) << "  Wall dir = V, wall pos (game)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
         LOG_INFO(DOM) << "                wall pos (plugin) = [" << (int)(lastActWallPos.x) << ", " << (lastActWallPos.y - 1) << "]";
         PlaceVertWallByOpponent({(lastActWallPos.x), (int8_t)(lastActWallPos.y - 1)});
      }

      UpdateMyPos({myPos.x, myPos.y});
      UpdateOpponentsPos({oppPos.x, oppPos.y});
      UpdateMyPossibleMoves();
      debug_PrintMyPossibleMoves(GetBoard());
      LOG_INFO(DOM) << "  Opp minpath = " << (int)FindMinPathLen(OPPONENT) << ", (debug_flags = " << (int)debug_GetFlags() << ")";
      LOG_INFO(DOM) << "  My minpath = " << (int)FindMinPathLen(ME) << ", (debug_flags = " << (int)debug_GetFlags() << ")";
      LOG_INFO(DOM) << "";
      debug_PrintBoard(GetBoard());      

#if (RUN_TESTS)
      test_1_CheckInitialBoardStructure(GetBoard());
      test_2_PlaceOneHorizWallThatIsNotOnTheBorder(GetBoard());
      test_3_UndoLastWall(GetBoard());
      test_4_PlaceTwoConsecutiveHorizWalls(GetBoard());
      test_5_UndoLastTwoWallsOneByOne(GetBoard());
      test_6_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(GetBoard());
      test_7_Place2VertWallsAndOneHorizWallAndThenUndoAll(GetBoard());
      test_8_PlaceAndUndoGroupsOf3Walls(GetBoard());
#endif
      LOG_INFO(DOM) << "---------------------------------------------------------------";
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
