#include "MB6_player.h"
#include <queue>

using namespace qcore::literals;
using namespace std::chrono_literals;


#define ACTION_TYPE_TO_STRING(actionType)    (actionType == qcore::ActionType::Invalid ? "Invalid" : \
                                                (actionType == qcore::ActionType::Move ? "Move" : "Wall"))

#define WALL_ORIENTATION_TO_STRING(wallOr)   (wallOr == qcore::Orientation::Vertical ? "V" : "H")

#define UNDEF_POS                            { (-1), (-1) }
#define INFINITE_LEN                         (0xFF)


namespace qplugin
{
   const char * const DOM = "MB6";
   uint8_t MB6_Board::mEnemyBaseRow[2]; // init static member of class. Will be populated once player IDs are known.

   MB6_Player::MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** This function defines the behaviour of the player. It is called by the game when it is my turn, and it 
    *  needs to end with a call to one of the action functions: move(...), placeWall(...).
    */      
   void MB6_Player::doNextMove()
   {

      static MB6_Board board;
      static MB6_Logger logger;

      // get game info ----------------------------------------------------------------------------------------------------------------------
      qcore::PlayerId      myID           = getId(); // 0 (if I am the first player to move) or 1
      qcore::PlayerId      oppID          = (myID ? 0 : 1);
      qcore::Position      myPos          = getPosition();
      qcore::BoardStatePtr boardState     = getBoardState();
      qcore::PlayerState   oppState       = boardState->getPlayers(oppID).at(oppID);
      qcore::Position      oppPos         = (oppState.position).rotate(2); // always rotate, as player positions are relative
      qcore::PlayerAction  lastAct        = boardState->getLastAction();
      qcore::ActionType    lastActType    = lastAct.actionType;
      qcore::Orientation   lastActWallOr  = lastAct.wallState.orientation;
      qcore::Position      lastActWallPos = (myID == 0 ? lastAct.wallState.position : // (myID == 0) means I am the first to move
                                                CoreAbsToRelWallPos(lastAct.wallState.position, lastActWallOr)); // only convert when I am 2nd to move
      uint8_t              myWallsLeft    = getWallsLeft();
      uint8_t              oppWallsLeft   = oppState.wallsLeft;

      // update board structure -------------------------------------------------------------------------------------------------------------
      board.UpdatePos(myID, CoreToPluginPlayerPos(myPos));
      board.UpdatePos(oppID, CoreToPluginPlayerPos(oppPos));

      board.mEnemyBaseRow[myID] = 1;  // I always start at the bottom, so enemy base for me is at the top
      board.mEnemyBaseRow[oppID] = 9; // opponent always starts at the top, so enemy base for him is at the bottom

      qcore::Position lastActWallPluginPos{ 0, 0 };
      if (lastActType == qcore::ActionType::Wall)
      {
         lastActWallPluginPos = CoreToPluginWallPos(lastActWallPos, lastActWallOr); 
         board.PlaceWall(lastActWallPluginPos, lastActWallOr);
      }

      bool pathForMeExists = board.ComputeMinPath(myID);
      bool pathForOppExists = board.ComputeMinPath(oppID);

      // log info ---------------------------------------------------------------------------------------------------------------------------
      LOG_INFO(DOM) << "  >  Turn count = " << turn;
      LOG_INFO(DOM) << "  >  Me  (" << (int)myID << ") pos = [" << (int)myPos.x << ", " << (int)myPos.y << "], walls = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "  >  Opp (" << (int)oppID << ") pos = [" << (int)oppPos.x << ", " << (int)oppPos.y << "], walls = " << (int)oppWallsLeft;
      LOG_INFO(DOM) << "  >  Last act = " << ACTION_TYPE_TO_STRING(lastActType);

      if (lastActType == qcore::ActionType::Wall)
      {  
         LOG_INFO(DOM) << "  >     [core]:   " << WALL_ORIENTATION_TO_STRING(lastActWallOr) << "[" << (int)lastActWallPos.x << ", " << (int)lastActWallPos.y << "]";
         LOG_INFO(DOM) << "  >     [plugin]: " << WALL_ORIENTATION_TO_STRING(lastActWallOr) << "[" << (int)lastActWallPluginPos.x << ", " << (int)lastActWallPluginPos.y << "]";
      }

      LOG_INFO(DOM) << "  >  My min path = " << (pathForMeExists ? (int)board.GetMinPath(myID) : INFINITE_LEN);
      LOG_INFO(DOM) << "  >  Opp min path = " << (pathForOppExists ? (int)board.GetMinPath(oppID) : INFINITE_LEN);

      logger.LogBoard(board, myID);

      // perform dummy move ----------------------------------------------------------------------------------------------------------------
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

   qcore::Position MB6_Player::CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation)   
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

   qcore::Position MB6_Player::CoreToPluginWallPos(qcore::Position corePos, qcore::Orientation orientation)
   {
      return (orientation == qcore::Orientation::Vertical ? qcore::Position{++corePos.x, corePos.y} : 
                                                               qcore::Position{corePos.x, ++corePos.y});
   }


   qcore::Position MB6_Player::CoreToPluginPlayerPos(qcore::Position corePos)
   {
      return qcore::Position{++corePos.x, ++corePos.y};
   }

   void MB6_Board::UpdatePos(qcore::PlayerId id, qcore::Position pos)
   {
      mPlayerPos[id] = pos;
   }

   void MB6_Board::PlaceWall(qcore::Position pos, qcore::Orientation orientation)
   {
      if (orientation == qcore::Orientation::Horizontal)
      {
         mBoard[pos.x][pos.y] |= 1;
         mBoard[pos.x][pos.y + 1] |= 1;
      }
      else
      {
         mBoard[pos.x][pos.y] |= 2;
         mBoard[pos.x + 1][pos.y] |= 2;
      }
   }

   bool MB6_Board::ComputeMinPath(qcore::PlayerId id)
   {
      // check if player is in enemy base
      if (mPlayerPos[id].x == mEnemyBaseRow[id])
      {
         // min path found and its length is 0
         mMinPathLen[id] = 0;
         return true;
      }

      // define a structure for saving information for a tile when it is reached:
      typedef struct
      {
         qcore::Position tilePos = UNDEF_POS;      // current tile position
         qcore::Position prevTilePos = UNDEF_POS;  // position of the tile we came from
         uint8_t pathLen = 0;                      // path length from player to current tile
      }TileInfo_t;

      // save info for each tile to this array as the tiles are being reached
      TileInfo_t visitedTiles[qcore::BOARD_SIZE + 1][qcore::BOARD_SIZE + 1]; // initialized with the values from type definition
      std::queue<TileInfo_t> q;

      // start with player's position (no previous tile and path length 0)
      q.push(TileInfo_t{ mPlayerPos[id], UNDEF_POS, 0 });

      // breadth-first-search: (if queue ever gets empty then there is no valid path from player to enemy base)
      while (!q.empty())
      {
         TileInfo_t tile = q.front();
         q.pop();

         auto NotVisited = [&](qcore::Position pos) { return (visitedTiles[pos.x][pos.y].tilePos == qcore::Position(UNDEF_POS)); };

         if (NotVisited(tile.tilePos))
         {
            //  save info about visited tile
            visitedTiles[tile.tilePos.x][tile.tilePos.y] = tile;

            // check if enemy base was reached
            if(tile.tilePos.x == mEnemyBaseRow[id])
            {
               // found min path
               mMinPathLen[id] = tile.pathLen;
               return true;
            }

            // go through neighbour tiles and add them to the queue if the min path to them was not found yet
            // 1. up:
            if (!HasWallAbove(tile.tilePos) && NotVisited(tile.tilePos - 1_x))
            {
               q.push(TileInfo_t{ (tile.tilePos - 1_x), tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 2. left:
            if (!HasWallToLeft(tile.tilePos) && NotVisited(tile.tilePos - 1_y))
            {
               q.push(TileInfo_t{ (tile.tilePos - 1_y), tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 3. right:
            if (!HasWallToRight(tile.tilePos) && NotVisited(tile.tilePos + 1_y))
            {
               q.push(TileInfo_t{ (tile.tilePos + 1_y), tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 4. down:
            if (!HasWallBelow(tile.tilePos) && NotVisited(tile.tilePos + 1_x))
            {
               q.push(TileInfo_t{ (tile.tilePos + 1_x), tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }
         }
      }

      // q is empty => no path exists
      mMinPathLen[id] = INFINITE_LEN;
      return false;
   }

   uint8_t MB6_Board::GetMinPath(qcore::PlayerId id)
   {
      return mMinPathLen[id];
   }

   bool MB6_Board::HasWallAbove(qcore::Position pos)
   {
      return (mBoard[pos.x - 1][pos.y] & 1);
   }

   bool MB6_Board::HasWallBelow(qcore::Position pos)
   {
      return (mBoard[pos.x][pos.y] & 1);
   }

   bool MB6_Board::HasWallToLeft(qcore::Position pos)
   {
      return (mBoard[pos.x][pos.y - 1] & 2);
   }

   bool MB6_Board::HasWallToRight(qcore::Position pos)
   {
      return (mBoard[pos.x][pos.y] & 2);
   }

   void MB6_Logger::LogBoard(MB6_Board board, qcore::PlayerId myID)
   {
      char map[qcore::BOARD_MAP_SIZE][qcore::BOARD_MAP_SIZE] = { 0 };

      // add walls to map
      for (int8_t i = 1; i <= qcore::BOARD_SIZE; i++)
      {
         for (int8_t j = 1; j <= qcore::BOARD_SIZE; j++)
         {
            // convert board coords to map coords
            qcore::Position mapPos = BoardToMapPosition(qcore::Position{i, j});

            // start with an empty tile
            map[mapPos.x][mapPos.y] = ' ';

            // add horizontal wall below if applicable
            if (i < qcore::BOARD_SIZE)
            {
               map[mapPos.x + 1][mapPos.y] = (board.mBoard[i][j] & 1 ? '=' : ' ');
            }

            // add vertical wall to the right if applicable
            if (j < qcore::BOARD_SIZE)
            {
               map[mapPos.x][mapPos.y + 1] = (board.mBoard[i][j] & 2 ? '|' : ' ');
            }

            // add a dot at the intersection of vertical and horizontal wall lines
            if ((i < qcore::BOARD_SIZE) && (j < qcore::BOARD_SIZE))
            {
               map[mapPos.x + 1][mapPos.y + 1] = '.';
            }
         }
      }

      // add player positions to the map
      qcore::Position posPlayer0 = BoardToMapPosition(board.mPlayerPos[0]);
      qcore::Position posPlayer1 = BoardToMapPosition(board.mPlayerPos[1]);
      map[posPlayer0.x][posPlayer0.y] = '0';
      map[posPlayer1.x][posPlayer1.y] = '1';

      // log map
      LOG_INFO(DOM) << "  >  ========================== ";
      for (int i = 0; i < qcore::BOARD_MAP_SIZE; i++)
      {
         ClearBuff();
         int bIndex = 0;

         // start of a new line
         mBuff[bIndex++] = ' '; mBuff[bIndex++] = ' '; mBuff[bIndex++] = '>'; mBuff[bIndex++] = ' '; mBuff[bIndex++] = '|';

         for (int j = 0; j < qcore::BOARD_MAP_SIZE; j++)
         {
            if ((j % 2) == 0)
            {
               if (map[i][j] != '0' && map[i][j] != '1')
               {
                  // duplicate free tiles and horiz wall tiles
                  mBuff[bIndex++] = map[i][j];
                  mBuff[bIndex++] = map[i][j];
               }
               else
               {
                  // set player tiles to something more meaningful
                  if (map[i][j] - 48 == myID)
                  {
                     mBuff[bIndex++] = 'M';
                     mBuff[bIndex++] = 'E';
                  }
                  else
                  {
                     mBuff[bIndex++] = 'O';
                     mBuff[bIndex++] = 'P';
                  }
               }
            }
            else
            {
               // vertical wall tiles and dots
               mBuff[bIndex++] = map[i][j];
            }
         }

         // end the line with a vertical border
         mBuff[bIndex++] = '|';

         LOG_INFO(DOM) << mBuff;
      }
      LOG_INFO(DOM) << "  >  ========================== ";
   }

   qcore::Position MB6_Logger::BoardToMapPosition(qcore::Position pos)
   {
      return qcore::Position{ (int8_t)((pos.x - 1) * 2), (int8_t)((pos.y - 1) * 2) };
   }

   void MB6_Logger::ClearBuff(void)
   {
      memset(mBuff, 0, sizeof(mBuff));
   }

} // end namespace
