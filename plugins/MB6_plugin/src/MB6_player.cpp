#include "MB6_player.h"
#include "MB6_logger.h"
#include <queue>

using namespace qcore::literals;
using namespace std::chrono_literals;
namespace qplugin
{
   // Initialize static members, value will be assigned at runtime
   qcore::PlayerId MB6_Player::_mMyId = UNDEF_ID;
   qcore::PlayerId MB6_Player::_mOppId = UNDEF_ID;

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
      // Create board (once)
      static MB6_Board board;

      // Get Player Ids (once). My Id can be 0 (if I am the first player to move) or 1.
      if (mTurnCount == 0)
      {
         _mMyId  = getId(); 
         _mOppId = (_mMyId ? 0 : 1);
      }

      // Update Turn Count
      mTurnCount++;

      // Get game info
      auto const myWallsLeft        = getWallsLeft();
      auto const myPos              = getPosition();
      auto const boardState         = getBoardState();
      auto const oppState           = boardState->getPlayers(_mOppId).at(_mOppId);
      auto const oppWallsLeft       = oppState.wallsLeft;
      auto const oppPos             = (oppState.position).rotate(2); // always rotate, as player positions are relative
      auto const lastAct            = boardState->getLastAction();
      auto const lastWallOri        = lastAct.wallState.orientation;
      auto const coreAbsLastWallPos = lastAct.wallState.position; // absolute wall position in the game core
      auto const coreRelLastWallPos = (_mMyId == 0 ? // relative wall position in the game core
                                          coreAbsLastWallPos : CoreAbsToRelWallPos(coreAbsLastWallPos, lastWallOri));
      auto const pluginLastWallPos  = (lastAct.actionType == qcore::ActionType::Wall ? // wall position in the plugin
                                          CoreToPluginWallPos(coreRelLastWallPos, lastWallOri) : UNDEF_POS);

      // Update board structure
      board.UpdatePlayerPos(_mMyId, myPos);
      board.UpdatePlayerPos(_mOppId, oppPos);
      board.UpdateWallsLeft(_mMyId, myWallsLeft);
      board.UpdateWallsLeft(_mOppId, oppWallsLeft);
      if (lastAct.actionType == qcore::ActionType::Wall)
      {
         board.PlaceWall(pluginLastWallPos, lastWallOri);
      }

      // Compute minimum paths
      uint8_t myMinPath = board.GetMinPath(_mMyId);
      uint8_t oppMinPath = board.GetMinPath(_mOppId);

      // Log information
      static MB6_Logger logger;
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

      ///////////////////////////////////////////////////////////////////////////////////////////////////////////
      // perform dummy move
      auto myPosition = getPosition() * 2;
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
         if (myPosition == p)
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

      // LOG_WARN(DOM) << "Something went wrong. Making a random move.";
      move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }

   qcore::Position MB6_Player::CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      return (ori == qcore::Orientation::Vertical ? pos - 1_y : pos - 1_x);
   }

   qcore::Position MB6_Player::CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      // flip around the board center, on both axis
      auto FlippedPos = qcore::Position(qcore::BOARD_SIZE, qcore::BOARD_SIZE) - pos;
      
      // adjust for the wrong end of the wall
      qcore::Position Adj = { (int8_t)(ori == qcore::Orientation::Vertical ? 2 : 0), (int8_t)(ori == qcore::Orientation::Horizontal ? 2 : 0) };

      // subtract adjustments
      return (FlippedPos - Adj);
   }

} // end namespace