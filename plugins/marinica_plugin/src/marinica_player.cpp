#include "marinica_player.h"
#include "plugin_core_interface.h"
#include "QcoreUtil.h"
#include "board.h"
#include "debug.h"
#include "tests.h"
#include "min_path.h"
#include "minimax.h"
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
      int turn = (turnCount++);

      static Board_t* board;

      if (turn == 0)
      {
         // only on the first function entry
         board = NewDefaultBoard();

         #if (RUN_TESTS)
            RunAllTests(board);
         #endif
      }


      // get game info ----------------------------------------------------------------------------------------------------------------------
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
                                                CoreAbsToRelWallPos(lastAct.wallState.position, lastActWallOr)); // only convert when I am 2nd to move
      uint8_t              myWallsLeft    = getWallsLeft();
      uint8_t              oppWallsLeft   = oppState.wallsLeft;


      // update board structure -------------------------------------------------------------------------------------------------------------
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

      bool foundMinPathOpp;
      uint8_t minPathOpp = FindMinPathLen(board, OPPONENT, &foundMinPathOpp);

      bool foundMinPathMe;
      uint8_t minPathMe = FindMinPathLen(board, ME, &foundMinPathMe);


      // log info ---------------------------------------------------------------------------------------------------------------------------
      LOG_INFO(DOM) << "---------------------------------------------------------------";
      LOG_INFO(DOM) << "  Turn count = " << turn;
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

      LOG_INFO(DOM) << "  My minpath =  " << (int)minPathMe << ", (found = " << (foundMinPathMe ? "true" : "false") << ")";
      LOG_INFO(DOM) << "  Opp minpath = " << (int)minPathOpp << ", (found = " << (foundMinPathOpp ? "true" : "false") << ")";
      LOG_INFO(DOM) << "";

      debug_PrintBoard(board);


      // Find best play ---------------------------------------------------------------------------------------------------------------------
      Minimax(board, ME, MINIMAX_LEVEL);
      Play_t bestPlay = GetBestPlayForLevel(MINIMAX_LEVEL);
      debug_PrintPlay(bestPlay);
      LOG_INFO(DOM) << "---------------------------------------------------------------";

      // Perform best play ------------------------------------------------------------------------------------------------------------------
      if (bestPlay.action == PLACE_WALL)
      {
         // place wall in the plugin logic
         PlaceWall(board, ME, GetWallByPosAndOrientation(board, bestPlay.wall->pos, bestPlay.wall->orientation));

         // place wall in the game core
         placeWall(qcore::WallState{PluginToCoreWallPos(bestPlay.wall->pos, bestPlay.wall->orientation), 
                     PluginToCoreWallOrientation(bestPlay.wall->orientation)});

         return;
      }
      else if (bestPlay.action == MAKE_MOVE)
      {
         // make move in the plugin logic
         MakeMove(board, ME, bestPlay.moveID);

         // make move in the game core
         move((myPos.x + board->moves[ME][bestPlay.moveID].xDiff), 
               (myPos.y + board->moves[ME][bestPlay.moveID].yDiff));

         return;
      }
      else // something went wrong, perform dummy move --------------------------------------------------------------------------------------
      {
         LOG_INFO(DOM) << "Player " << (int)getId() << " has f*cked up.. Fallback on dummy player... ";

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
   }

} // namespace qplugin
