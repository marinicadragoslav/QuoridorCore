#include "marinica_player.h"
#include "QcoreUtil.h"
#include "board.h"
#include "debug.h"
#include "tests.h"
#include "min_path.h"
#include <thread>

#define POS_INFINITY 0xFFFFFF
#define NEG_INFINITY (-0xFFFFFF)
#define I_WIN 0x0FFFFF
#define OPP_WINS (-0x0FFFFF)

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

   static int minimax(Board_t* board, Player_t player, uint8_t level)
   {
      if (HasWon(ME))
      {
         return I_WIN;
      }
      else if (HasWon(OPPONENT))
      {
         return OPP_WINS;
      }
      else if (level == 0)
      {
         uint8_t oppMinPath = FindMinPathLen(OPPONENT);
         uint16_t oppFlags = debug_GetReachedGoalTiles();
         uint8_t myMinPath = FindMinPathLen(ME);
         uint16_t myFlags = debug_GetReachedGoalTiles();

         if (myFlags == 0 || oppFlags == 0)
         {
            return NEG_INFINITY; // this will be checked and discarded if too large/small
         }

         return (oppMinPath - myMinPath);
      }
      else
      {
         int score = (player == ME ? NEG_INFINITY : POS_INFINITY);
         int eval;
         
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
                     PlaceWall(player, wall);

                     if (player == ME)
                     {
                        debug_PrintTestMessage("Minimax: ME placed wall:");
                     }
                     else
                     {
                        debug_PrintTestMessage("Minimax: OPP placed wall:");
                     }                     
                     debug_PrintWall(wall);

                     eval = minimax(board, board->otherPlayer[player], level - 1);
                     if (eval > I_WIN || eval < OPP_WINS)
                     {
                        debug_PrintTestMessage("Eval too large, discard");
                     }
                     else
                     {
                        debug_PrintMinimaxScore(eval);
                        score = (player == ME ? (eval > score ? eval : score) : (eval < score ? eval : score));
                     }

                     UndoWall(player, wall);
                  }
               }
            }
         }

         UpdatePossibleMoves(player);

         // go through moves
         for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
         {
            if (board->moves[player][moveID].isPossible)
            {
               MakeMove(player, (MoveID_t)moveID);
               if (player == ME)
               {
                  debug_PrintTestMessage("Minimax: ME moved:");
               }
               else
               {
                  debug_PrintTestMessage("Minimax: OPP moved:");
               }
               debug_PrintMove((MoveID_t)moveID);

               eval = minimax(board, board->otherPlayer[player], level - 1);

               if (eval > I_WIN || eval < OPP_WINS)
               {
                  debug_PrintTestMessage("Eval too large, discard");
               }
               else
               {
                  debug_PrintMinimaxScore(eval);
                  score = (player == ME ? (eval > score ? eval : score) : (eval < score ? eval : score));
               }              

               UndoMove(player, (MoveID_t)moveID);

               UpdatePossibleMoves(player);
            }
         }

         return score;
      }
   }

   static BestPlay_t FindBestPlay(Board_t* board, uint8_t level)
   {
      BestPlay_t bp = {WALL, MOVE_SOUTH, &(board->walls[V][BOARD_SZ - 2][BOARD_SZ - 2])}; // init with some values
      int max = NEG_INFINITY;
      int score;

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
                  PlaceWall(ME, wall);
                  debug_PrintTestMessage("FBP: Placed wall:");
                  debug_PrintWall(wall);

                  score = minimax(board, OPPONENT, level - 1);

                  if (score > I_WIN || score < OPP_WINS)
                  {
                     debug_PrintTestMessage("Eval too large, discard");
                  }
                  else
                  {
                     debug_PrintMinimaxScore(score);
                     if (score > max)
                     {
                        max = score;
                        bp.action = WALL;
                        bp.wall = wall;
                        debug_PrintTestMessage("New BP: ");
                        debug_PrintBestPlay(bp);
                     }
                  }                 

                  UndoWall(ME, wall);
               }
            }
         }
      }

      UpdatePossibleMoves(ME);

      // go through moves
      for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
      {
         if (board->moves[ME][moveID].isPossible)
         {
            MakeMove(ME, (MoveID_t)moveID);
            debug_PrintTestMessage("FBP: Made move:");
            debug_PrintMove((MoveID_t)moveID);

            score = minimax(board, OPPONENT, level - 1);
            if (score > I_WIN || score < OPP_WINS)
            {
               debug_PrintTestMessage("Eval too large, discard");
            }
            else
            {
               debug_PrintMinimaxScore(score);
               if (score > max)
               {
                  max = score;
                  bp.action = MOVE;
                  bp.moveID = (MoveID_t)moveID;
                  debug_PrintTestMessage("New BP: ");
                  debug_PrintBestPlay(bp);
               }
            }

            UndoMove(ME, (MoveID_t)moveID);

            UpdatePossibleMoves(ME);
         }
      }

      return bp;
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
         InitBoard();
         board = GetBoard(); // debugging only
         
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
      UpdatePos(ME, {myPos.x, myPos.y});
      UpdatePos(OPPONENT, {oppPos.x, oppPos.y});

      if (lastActType == qcore::ActionType::Wall)
      {  
         Position_t wallPos = CoreToPluginWallPos(lastActWallPos, lastActWallOr);
         Orientation_t wallOr = CoreToPluginWallOrientation(lastActWallOr);

         PlaceWall(OPPONENT, GetWall(wallPos, wallOr));
      }

      UpdatePossibleMoves(ME);
      UpdatePossibleMoves(OPPONENT);

      uint8_t minPathOpp = FindMinPathLen(OPPONENT);
      uint16_t debugFlagsOpp = debug_GetReachedGoalTiles(); // only for debugging
      uint8_t minPathMe = FindMinPathLen(ME);
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
      BestPlay_t bp = FindBestPlay(board, 3);
      debug_PrintBestPlay(bp);
      LOG_INFO(DOM) << "---------------------------------------------------------------";

      if (bp.action == WALL && bp.wall->orientation == H)
      {
         int8_t coreWallPosX = bp.wall->pos.x + 1;
         int8_t coreWallPosY = bp.wall->pos.y;
         PlaceWall(ME, GetWall(bp.wall->pos, H));
         placeWall(coreWallPosX, coreWallPosY, qcore::Orientation::Horizontal);
      }
      else if (bp.action == WALL && bp.wall->orientation == V)
      {
         int8_t coreWallPosX = bp.wall->pos.x;
         int8_t coreWallPosY = bp.wall->pos.y + 1;
         PlaceWall(ME, GetWall(bp.wall->pos, V));
         placeWall(coreWallPosX, coreWallPosY, qcore::Orientation::Vertical);
      }
      else if (bp.action == MOVE)
      {
         MakeMove(ME, bp.moveID);
         move(myPos.x + board->moves[ME][bp.moveID].xDiff, myPos.y + board->moves[ME][bp.moveID].yDiff);
      }
      turnCount++;
      return;

      /*
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
      */
   }

} // namespace qplugin
