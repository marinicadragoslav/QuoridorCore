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
    *  Plugin player is called "ME", the other player is "OPPONENT".
    *  The board will always be viewed from my perspective, with me on the bottom (last row).
    *  Opponent's position (as given by the game) will always need converting to my prespective.
    *  Last wall position (as given by the game) will only need converting to my perspective when I am the second player.
    *  The game graph is represented by a matrix of tiles with each tile linked to its neighbours:
    *                         
    *                                                                Placing a wall renders
    *                  NULL                                          the corresponding links NULL:
    *                   ^                                          
    *                0  | north     1          2                    0           1          2       
    *             ------|------------------------              -------------------------------               
    *             |     |     |           |                    |           |           |
    *          0  |           |           |                    |           |           |   
    *            west      east           |                 0  |  NULL     |    NULL   |
    *    NULL <-------  |  ------->       |                    |    ^      |    ^      |
    *             |     |     |           |                    |    | |    |    | |    |
    *             ------|------------------------              -xxxx|x|xxxxxxxxx|x|xxxx--------
    *             |     v     |           |                    |    | |    |      |    |
    *             |    south  |           |                    |      v    |      v    |
    *          1  |           |           |                 1  |     NULL  |     NULL  |
    *             |           |           |                    |           |           |
    *             |           |           |
    *             -------------------------------
    *             |           |           |
    *             |           |           |
    *          2  |           |           |
    * 
    * 
    *  TO BE CONTINUED...    
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

   // Convert core wall position to plugin wall position (both type and value)
   static Position_t CoreToPluginWallPos(qcore::Position gameWallPos, qcore::Orientation orientation)
   {
      Position_t pluginWallPos;

      if (orientation == qcore::Orientation::Horizontal)
      {
         pluginWallPos.x = gameWallPos.x - 1;
         pluginWallPos.y = gameWallPos.y;
      }
      else
      {           
         pluginWallPos.x = gameWallPos.x;
         pluginWallPos.y = gameWallPos.y - 1;
      }

      return pluginWallPos;
   }

   // Convert core wall orientation type to plugin wall orientation type
   static Orientation_t CoreToPluginWallOrientation(qcore::Orientation orientation)
   {
      return (orientation == qcore::Orientation::Horizontal ? H : V);
   }

   static void SpeedTest(Board_t* board, uint8_t level)
   {
      if (level == 0)
      {
         return;
      }

      // go through walls
      for (int o = H; o <= V; o++)
      {
         for (int i = 0; i < BOARD_SZ - 1; i++)
         {
            for (int j = 0; j < BOARD_SZ - 1; j++)
            {
               Wall_t* wall = &(board->walls[o][i][j]);

               if (wall->permission == WALL_PERMITTED)
               {
                  PlaceWall(board, ME, wall);
                  // debug_PrintWall(wall);

                  SpeedTest(board, level - 1);

                  UndoWall(board, ME, wall);
               }
            }
         }
      }

      UpdatePossibleMoves(board, ME);

      // go through moves
      for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
      {
         if (board->moves[ME][moveID].isPossible)
         {
            MakeMove(board, ME, (MoveID_t)moveID);
            // debug_PrintMove((MoveID_t)moveID);

            SpeedTest(board, level - 1);

            UndoMove(board, ME, (MoveID_t)moveID);
            UpdatePossibleMoves(board, ME);
         }
      }
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
      static Board_t* board;

      if (turnCount == 0)
      {
         board = NewDefaultBoard();
         
#if (RUN_TESTS)
      test_1_CheckInitialBoardStructure(board);
      test_2_PlaceThenUndoOneHorizWallThatIsNotOnTheBorder(board);
      test_3_PlaceThenUndoOneVertWallThatIsNotOnTheBorder(board);
      test_4_PlaceThenUndoOneHorizWallThatIsOnTheBorder(board);
      test_5_PlaceThenUndoOneVertWallThatIsOnTheBorder(board);
      test_6_PlaceTwoConsecutiveHorizWallsAndUndoThem(board);
      test_7_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(board);
      test_8_Place2VertWallsAndOneHorizWallAndThenUndoAll(board);
      test_9_PlaceAndUndoGroupsOf3Walls(board);
      test_10_MinPathAndPossibleMoves();
      test_11_MinPathAndPossibleMoves();
      test_12_MinPathAndPossibleMoves();
      test_13_MinPathAndPossibleMoves();
      test_14_MinPathAndPossibleMoves();
      test_15_MinPathAndPossibleMoves();
      test_16_MinPathAndPossibleMoves();
      test_17_MinPathAndPossibleMoves();
      test_18_MinPathAndPossibleMoves();
      test_19_MinPathAndPossibleMoves();
      test_20_MinPathAndPossibleMoves();
      test_21_MinPathAndPossibleMoves();
      test_22_MinPathAndPossibleMoves();
#endif
      }


      // get game info 
      uint8_t              myID           = getId(); // 0 (if I am the first player to move) or 1
      uint8_t              oppID          = (myID ? 0 : 1);
      qcore::Position      myPos          = getPosition();
      qcore::BoardStatePtr boardState     = getBoardState();
      qcore::PlayerState   oppState       = boardState->getPlayers(oppID).at(oppID);
      qcore::Position      oppPos         = (oppState.position).rotate(2); // always rotate, as player positions are relative
      qcore::PlayerAction  lastAct        = boardState->getLastAction();
      qcore::ActionType    lastActType    = lastAct.actionType;
      qcore::Orientation   lastActWallOr  = lastAct.wallState.orientation;
      qcore::Position      lastActWallPos = (myID == 0 ? lastAct.wallState.position : 
                                                AbsToRelWallPos(lastAct.wallState.position, lastActWallOr)); // only convert when I am 2nd to move
      uint8_t              myWallsLeft    = getWallsLeft();
      uint8_t              oppWallsLeft   = oppState.wallsLeft;


      // update board structure
      UpdatePos(board, ME, {myPos.x, myPos.y});
      UpdatePos(board, OPPONENT, {oppPos.x, oppPos.y});

      if (lastActType == qcore::ActionType::Wall)
      {  
         Position_t wallPos = CoreToPluginWallPos(lastActWallPos, lastActWallOr);
         Orientation_t wallOr = CoreToPluginWallOrientation(lastActWallOr);

         PlaceWall(board, OPPONENT, GetWallByPosAndOrientation(board, wallPos, wallOr));
      }

      UpdatePossibleMoves(board, ME);
      UpdatePossibleMoves(board, OPPONENT);

      uint8_t minPathOpp = FindMinPathLen(board, OPPONENT);
      uint16_t debugFlagsOpp = debug_GetReachedGoalTiles(); // only for debugging
      uint8_t minPathMe = FindMinPathLen(board, ME);
      uint16_t debugFlagsMe = debug_GetReachedGoalTiles(); // only for debugging


      // log info
      LOG_INFO(DOM) << "---------------------------------------------------------------";
      LOG_INFO(DOM) << "  Turn count = " << turnCount;
      LOG_INFO(DOM) << "  Me  (" << (int)myID << ") pos = [" << (int)myPos.x << ", " << (int)myPos.y << "], walls = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "  Opp (" << (int)oppID << ") pos = [" << (int)oppPos.x << ", " << (int)oppPos.y << "], walls = " << (int)oppWallsLeft;
      LOG_INFO(DOM) << "  Last act = " << (lastActType == qcore::ActionType::Invalid ? "Invalid" : 
                                                (lastActType == qcore::ActionType::Move ? "Move" : "Wall"));

      if (lastActType == qcore::ActionType::Wall)
      {         
         if (lastActWallOr == qcore::Orientation::Horizontal)
         {
            LOG_INFO(DOM) << "  Wall dir = H, wall pos (core)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
            LOG_INFO(DOM) << "                wall pos (plugin) = [" << (lastActWallPos.x - 1) << ", " << (int)lastActWallPos.y << "]";
         }
         else
         {
            LOG_INFO(DOM) << "  Wall dir = V, wall pos (core)   = [" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
            LOG_INFO(DOM) << "                wall pos (plugin) = [" << (int)(lastActWallPos.x) << ", " << (lastActWallPos.y - 1) << "]";
         }
      }     

      debug_PrintMyPossibleMoves(board);
      debug_PrintOppPossibleMoves(board);

      LOG_INFO(DOM) << "  My minpath =  " << (int)minPathMe << ", (debug_flags = " << (int)debugFlagsMe << ")";
      LOG_INFO(DOM) << "  Opp minpath = " << (int)minPathOpp << ", (debug_flags = " << (int)debugFlagsOpp << ")";
      LOG_INFO(DOM) << "";

      debug_PrintBoard(board); // includes the last computed min path


      // Test
      SpeedTest(board, 3);

      LOG_INFO(DOM) << "---------------------------------------------------------------";
      turnCount++;

      // ----------------------------------------------------------------------------
      LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking..";

      // Simulate more thinking
      // std::this_thread::sleep_for(1000ms);

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
