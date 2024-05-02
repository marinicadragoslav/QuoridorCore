#include "MB6_player.h"
#include "MB6_board.h"
#include "MB6_timer.h"
#include "MB6_logger.h"
#include <vector>

#define MINIMAX_DEPTH         3
#define MINIMAX_TIMEOUT_MS    4900

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace qplugin
{
   // Player construction
   MB6_Player::MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
      mTurnCount = 0;
   }

   /** This function defines the behaviour of the player. It is called by the game when it is my turn, and it 
    *  needs to end with a call to one of the action functions: move(...), placeWall(...)
    */
   void MB6_Player::doNextMove()
   {
      static MB6_Logger logger;
      static MB6_Timer timer;
      static MB6_Board board;

      // Get Player Ids once, on my first move. My Id is 0 if I am the first player to move, or 1 otherwise.
      if (mTurnCount == 0)
      {
         MB6_Board::_mMyId  = getId(); 
         MB6_Board::_mOppId = (MB6_Board::_mMyId ? 0 : 1);
      }

      // Update Turn Count
      mTurnCount++;

      // Start the timer
      timer.Reset(MINIMAX_TIMEOUT_MS);
      timer.Start();

      // Get game info
      auto const myWallsLeft        = getWallsLeft();
      auto const myPos              = getPosition();
      auto const boardState         = getBoardState();
      auto const oppState           = boardState->getPlayers(MB6_Board::_mOppId).at(MB6_Board::_mOppId);
      auto const oppWallsLeft       = oppState.wallsLeft;
      auto const oppPos             = (oppState.position).rotate(2); // always rotate, as player positions are relative
      auto const lastAct            = boardState->getLastAction();
      auto const lastWallOri        = lastAct.wallState.orientation;
      auto const coreAbsLastWallPos = lastAct.wallState.position; // absolute wall position in the game core
      auto const coreRelLastWallPos = (MB6_Board::_mMyId == 0 ? // relative wall position in the game core
                                          coreAbsLastWallPos : CoreAbsToRelWallPos(coreAbsLastWallPos, lastWallOri));
      auto const pluginLastWallPos  = (lastAct.actionType == qcore::ActionType::Wall ? // wall position in the plugin
                                          CoreToPluginWallPos(coreRelLastWallPos, lastWallOri) : UNDEFINED_POSITION);

      // Update board structure
      board.UpdatePlayerPos(MB6_Board::_mMyId, myPos);
      board.UpdatePlayerPos(MB6_Board::_mOppId, oppPos);
      board.UpdateWallsLeft(MB6_Board::_mMyId, myWallsLeft);
      board.UpdateWallsLeft(MB6_Board::_mOppId, oppWallsLeft);
      if (lastAct.actionType == qcore::ActionType::Wall)
      {
         board.PlaceWall(pluginLastWallPos, lastWallOri);
      }

      // Compute minimum paths
      uint8_t myMinPath = board.GetMinPath(MB6_Board::_mMyId);
      uint8_t oppMinPath = board.GetMinPath(MB6_Board::_mOppId);

      // Log information
      logger.LogTurnCount(mTurnCount);
      logger.LogMyInfo(board);
      logger.LogOppInfo(board);
      logger.LogLastActType(lastAct.actionType);
      if (lastAct.actionType == qcore::ActionType::Wall)
      {  
         logger.LogLastActWallInfo(lastWallOri, coreRelLastWallPos, pluginLastWallPos);
      }
      logger.LogMyMinPath(myMinPath);
      logger.LogOppMinPath(oppMinPath);
      logger.LogBoard(board);

      // Get best action (Minimax with alpha-beta pruning)
      bool timerHasElapsed = false;
      board.Minimax(MB6_Board::_mMyId, MINIMAX_DEPTH, NEG_INFINITY, POS_INFINITY, timer, true, timerHasElapsed);
      Action_t bestAct = board.GetBestAction(MINIMAX_DEPTH);
      logger.LogBestAction(bestAct);

      // Perform action
      if (bestAct.actionType == ACT_TYPE_MOVE)
      {
         move(bestAct.position);
      }
      else
      {
         qcore::Orientation ori = (bestAct.actionType == ACT_TYPE_H_WALL ? 
                                    qcore::Orientation::Horizontal : qcore::Orientation::Vertical);         
         board.PlaceWall(bestAct.position, ori);
         qcore::Position pos = PluginToCoreWallPos(bestAct.position, ori);
         placeWall(pos.x, pos.y, ori);
      }
   }

   qcore::Position MB6_Player::CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      return (ori == qcore::Orientation::Vertical ? pos - 1_y : pos - 1_x);
   }

   qcore::Position MB6_Player::PluginToCoreWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      return (ori == qcore::Orientation::Vertical ? pos + 1_y : pos + 1_x);
   }

   qcore::Position MB6_Player::CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      // flip around the board center, on both axis
      qcore::Position flipped = qcore::Position(qcore::BOARD_SIZE, qcore::BOARD_SIZE) - pos;

      // adjust for the wrong end of the wall
      qcore::Position relPos = flipped - qcore::Position((int8_t)(ori == qcore::Orientation::Vertical ? 2 : 0), 
                                                            (int8_t)(ori == qcore::Orientation::Horizontal ? 2 : 0));

      return relPos;
   }

   qcore::Position MB6_Player::CoreRelToAbsWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      // revert the adjustment for the wrong end of the wall
      qcore::Position flipped = pos + qcore::Position((int8_t)(ori == qcore::Orientation::Vertical ? 2 : 0), 
                                                         (int8_t)(ori == qcore::Orientation::Horizontal ? 2 : 0));

      // revert the flip around the board center, on both axis
      qcore::Position absPos = qcore::Position(qcore::BOARD_SIZE, qcore::BOARD_SIZE) - flipped;

      return absPos;
   }

} // end namespace