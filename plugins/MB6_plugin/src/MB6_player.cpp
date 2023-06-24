#include "MB6_player.h"
#include "MB6_logger.h"
#include <queue>

using namespace qcore::literals;
using namespace std::chrono_literals;

#define INFINITE_LEN  (0xFF)

namespace qplugin
{
   static qcore::Position _CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
   static qcore::Position _CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori);

   qcore::PlayerId MB6_Board::mMyID = 0; // initialize static class member; will be updated once, at runtime

   MB6_Player::MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
      mTurnCount = 0;
   }

   /** This function defines the behaviour of the player. It is called by the game when it is my turn, and it 
    *  needs to end with a call to one of the action functions: move(...), placeWall(...).
    */      
   void MB6_Player::doNextMove()
   {
      // GET GAME INFO ----------------------------------------------------------------------------------------------------------------------
      auto const myID               = getId(); // 0 (if I am the first player to move) or 1
      auto const oppID              = (myID ? 0 : 1);
      auto const myWallsLeft        = getWallsLeft();
      auto const myPos              = getPosition();
      auto const boardState         = getBoardState();
      auto const oppState           = boardState->getPlayers(oppID).at(oppID);
      auto const oppWallsLeft       = oppState.wallsLeft;
      auto const oppPos             = (oppState.position).rotate(2); // always rotate, as player positions are relative
      auto const lastAct            = boardState->getLastAction();
      auto const lastWallOri        = lastAct.wallState.orientation;
      auto const coreAbsLastWallPos = lastAct.wallState.position;
      auto const coreRelLastWallPos = (myID == 0 ? coreAbsLastWallPos : _CoreAbsToRelWallPos(coreAbsLastWallPos, lastWallOri));
      auto const pluginLastWallPos  = (lastAct.actionType == qcore::ActionType::Wall ? 
                                          _CoreToPluginWallPos(coreRelLastWallPos, lastWallOri) : UNDEF_POS);
      
      // CREATE BOARD (ONCE) ----------------------------------------------------------------------------------------------------------------
      static MB6_Board board;
      board.mMyID = myID; // static class member mMyID: only assign once, never changes for any board instance.

      // UPDATE BOARD STRUCTURE -------------------------------------------------------------------------------------------------------------
      board.UpdatePlayerPos(myID, myPos);
      board.UpdatePlayerPos(oppID, oppPos);

      board.UpdateWallsLeft(myID, myWallsLeft);
      board.UpdateWallsLeft(oppID, oppWallsLeft);
      
      if (lastAct.actionType == qcore::ActionType::Wall)
      {
         board.PlaceWall(pluginLastWallPos, lastWallOri);
      }

      bool pathForMeExists = board.ComputeMinPath(myID);
      bool pathForOppExists = board.ComputeMinPath(oppID);

      // log info ---------------------------------------------------------------------------------------------------------------------------
      static MB6_Logger logger;
      logger.LogTurnCount(mTurnCount);
      logger.LogMyInfo(board);
      logger.LogOppInfo(board);
      logger.LogLastActType(lastAct.actionType);      

      if (lastAct.actionType == qcore::ActionType::Wall)
      {  
         logger.LogLastActWallInfo(lastWallOri, coreRelLastWallPos, pluginLastWallPos);
      }

      logger.LogMyMinPath(pathForMeExists, board, INFINITE_LEN);
      logger.LogOppMinPath(pathForOppExists, board, INFINITE_LEN);

      logger.LogBoard(board, myID);

      // perform dummy move ----------------------------------------------------------------------------------------------------------------
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
   
   void MB6_Board::UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos)
   {
      mPlayerPos[id] = pos;
   }

   void MB6_Board::UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls)
   {
      mWallsLeft[id] = walls;
   }

   void MB6_Board::PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      qcore::Position south = pos + qcore::Direction::Down;
      qcore::Position east = pos + qcore::Direction::Right;
      qcore::Position southeast = south + qcore::Direction::Right;

      if (ori == qcore::Orientation::Horizontal)
      {
         mBoard[pos.x][pos.y] |= WALL_SOUTH_1;
         mBoard[east.x][east.y] |= WALL_SOUTH_2;
         mBoard[south.x][south.y] |= WALL_NORTH_1;
         mBoard[southeast.x][southeast.y] |= WALL_NORTH_2;
      }
      else
      {
         mBoard[pos.x][pos.y] |= WALL_EAST_1;
         mBoard[south.x][south.y] |= WALL_EAST_2;
         mBoard[east.x][east.y] |= WALL_WEST_1;
         mBoard[southeast.x][southeast.y] |= WALL_WEST_2;
      }
   }

   bool MB6_Board::ComputeMinPath(const qcore::PlayerId& id)
   {
      if (IsPosInPlayersEnemyBase(mPlayerPos[id], id))
      {
         mMinPathLen[id] = 0;
         return true;
      }
      
      // save info for each tile to this array as the tiles are being reached
      TileInfo_t visitedTiles[qcore::BOARD_SIZE][qcore::BOARD_SIZE]; // initialized at type definition

      // queue for tile information
      std::queue<TileInfo_t> q;

      // start with player's position (no previous tile and path length 0)
      q.push(TileInfo_t{ mPlayerPos[id], UNDEF_POS, 0 });

      // breadth-first-search: (if queue ever gets empty then there is no valid path from player to enemy base)
      while (!q.empty())
      {
         TileInfo_t tileInfo = q.front();
         q.pop();

         auto NotVisited = [&](const qcore::Position& pos) -> bool { return (visitedTiles[pos.x][pos.y].tilePos == UNDEF_POS); };

         if (NotVisited(tileInfo.tilePos))
         {
            // save info about visited tile
            visitedTiles[tileInfo.tilePos.x][tileInfo.tilePos.y] = tileInfo;

            // check if enemy base was reached
            if(IsPosInPlayersEnemyBase(tileInfo.tilePos, id))
            {
               // found min path
               mMinPathLen[id] = tileInfo.pathLen;

               // make path visible on the board (debug mode only)
               #if (DEBUG)
               MarkMinPathOnBoard(visitedTiles, tileInfo.tilePos, id);
               #endif

               return true;
            }

            // go through neighbour tiles and add them to the queue if the min path to them was not found yet
            for (auto& dir : std::vector<qcore::Direction>{qcore::Direction::Up, qcore::Direction::Left, qcore::Direction::Right, qcore::Direction::Down})
            {
               qcore::Position nextPos = tileInfo.tilePos + dir;
               if (!HasWallTowards(tileInfo.tilePos, dir) && NotVisited(nextPos))
               {
                  q.push(TileInfo_t{ nextPos, tileInfo.tilePos, (uint8_t)(tileInfo.pathLen + 1) });
               }
            }
         }
      }

      // q is empty => no path exists
      mMinPathLen[id] = INFINITE_LEN;
      return false;
   }

   uint8_t MB6_Board::GetMinPath(const qcore::PlayerId& id) const
   {
      return mMinPathLen[id];
   }

   bool MB6_Board::HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const
   {
      switch(dir)
      {
         case qcore::Direction::Up:
            return (mBoard[pos.x][pos.y] & WALL_NORTH);
         case qcore::Direction::Down:
            return (mBoard[pos.x][pos.y] & WALL_SOUTH);
         case qcore::Direction::Left:
            return (mBoard[pos.x][pos.y] & WALL_WEST);
         case qcore::Direction::Right:
            return (mBoard[pos.x][pos.y] & WALL_EAST);
         default:
            return false;
      }
   }

   #if (DEBUG)
   void MB6_Board::MarkMinPathOnBoard(const TileInfo_t visitedTiles[][qcore::BOARD_SIZE], const qcore::Position& startPos, const qcore::PlayerId& id)
   {
      qcore::Position pos = startPos;
      do
      {
         mBoard[pos.x][pos.y] |= (id == mMyID ? MARKER_MY_PATH : MARKER_OPP_PATH);
         pos = visitedTiles[pos.x][pos.y].prevTilePos;
      } while (not(pos == mPlayerPos[id])); // operator "!=" is not implemented for positions
   }
   #endif

   bool MB6_Board::IsPosInPlayersEnemyBase(const qcore::Position& pos, const qcore::PlayerId& id) const
   {
      // row #0 is enemy base for me, row #8 is enemy base for opponent
      return ((id == mMyID && pos.x == 0) || (id != mMyID && pos.x == 8)); 
   }

   static qcore::Position _CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      return (ori == qcore::Orientation::Vertical ? pos - 1_y : pos - 1_x);
   }

   static qcore::Position _CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      // flip around the board center on both axis
      auto ret = qcore::Position(qcore::BOARD_SIZE, qcore::BOARD_SIZE) - pos;
      
      // compute adjustments for the wrong end of the wall
      uint8_t xAdj = (ori == qcore::Orientation::Vertical ? 2 : 0);
      uint8_t yAdj = (ori == qcore::Orientation::Horizontal ? 2 : 0);

      // subtract adjustments
      return (ret - qcore::Position(xAdj, yAdj));
   }

} // end namespace