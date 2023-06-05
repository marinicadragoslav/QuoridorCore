#include "MB6_player.h"
#include <queue>

using namespace qcore::literals;
using namespace std::chrono_literals;

#define UNDEF_POS       qcore::Position(-0xF, -0xF)
#define INFINITE_LEN    (0xFF)

namespace qplugin
{
   static qcore::Position CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
   static qcore::Position CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori);

   const char * const DOM = "MB6";
   qcore::PlayerId MB6_Board::mMyID = 0; // will be updated when known

   MB6_Player::MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
      turn = 0;
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
      auto const coreRelLastWallPos = (myID == 0 ? coreAbsLastWallPos : CoreAbsToRelWallPos(coreAbsLastWallPos, lastWallOri));
      auto const pluginLastWallPos  = (lastAct.actionType == qcore::ActionType::Wall ? 
                                          CoreToPluginWallPos(coreRelLastWallPos, lastWallOri) : UNDEF_POS);
      
      // CREATE BOARD (ONCE) ----------------------------------------------------------------------------------------------------------------
      static MB6_Board board;
      board.mMyID = myID; // static class member mMyID: only assign once, never changes for any board instance.

      // UPDATE BOARD STRUCTURE -------------------------------------------------------------------------------------------------------------
      board.UpdatePlayerPos(myID, myPos);
      board.UpdatePlayerPos(oppID, oppPos);
      
      if (lastAct.actionType == qcore::ActionType::Wall)
      {
         board.PlaceWall(pluginLastWallPos, lastWallOri);
      }

      bool pathForMeExists = board.ComputeMinPath(myID);
      bool pathForOppExists = board.ComputeMinPath(oppID);

      // log info ---------------------------------------------------------------------------------------------------------------------------
      LOG_INFO(DOM) << "  >  Turn count = " << (turn++);
      LOG_INFO(DOM) << "  >  Me  (" << (int)myID << ") pos = [" << (int)myPos.x << ", " << (int)myPos.y << "], walls = " << (int)myWallsLeft;
      LOG_INFO(DOM) << "  >  Opp (" << (int)oppID << ") pos = [" << (int)oppPos.x << ", " << (int)oppPos.y << "], walls = " << (int)oppWallsLeft;
      LOG_INFO(DOM) << "  >  Last act = " << (lastAct.actionType == qcore::ActionType::Move ? "Move" :
                                                (lastAct.actionType == qcore::ActionType::Wall ? "Wall" : "Invalid"));

      if (lastAct.actionType == qcore::ActionType::Wall)
      {  
         LOG_INFO(DOM) << "  >     [" << (lastWallOri == qcore::Orientation::Horizontal ? "Horizontal" : "Vertical") << "]";
         LOG_INFO(DOM) << "  >     [Core pos]:   " << "[" << (int)coreRelLastWallPos.x << ", " << (int)coreRelLastWallPos.y << "]";
         LOG_INFO(DOM) << "  >     [Plugin pos]: " << "[" << (int)(pluginLastWallPos.x) << ", " << (int)(pluginLastWallPos.y) << "]";
      }

      LOG_INFO(DOM) << "  >  My min path = " << (pathForMeExists ? (int)board.GetMinPath(myID) : INFINITE_LEN);
      LOG_INFO(DOM) << "  >  Opp min path = " << (pathForOppExists ? (int)board.GetMinPath(oppID) : INFINITE_LEN);

      static MB6_Logger logger;
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

      LOG_WARN(DOM) << "Something went wrong. Making a random move.";
      move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }
   
   void MB6_Board::UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos)
   {
      mPlayerPos[id] = pos;
   }

   void MB6_Board::PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      if (ori == qcore::Orientation::Horizontal)
      {
         mBoard[pos.x][pos.y] |= 4; // place below current tile (4 = horiz wall below, first segment)
         mBoard[pos.x][pos.y + 1] |= 8; // place below eastern neighbour (8 = horiz wall below, second segment)
         mBoard[pos.x + 1][pos.y] |= 1; // place above southern neighbour (1 = horiz wall above, first segment)
         mBoard[pos.x + 1][pos.y + 1] |= 2; // place above south-eastern neighbour (2 = horiz wall above, second segment)
      }
      else
      {
         mBoard[pos.x][pos.y] |= 64; // place to the right of current tile (64 = vert wall to the right, first segment)
         mBoard[pos.x + 1][pos.y] |= 128; // place to the right of southern neighbour (128 = vert wall to the right, second segment)
         mBoard[pos.x][pos.y + 1] |= 16; // place to the left of eastern neighbour (16 = vert wall to the left, first segment)
         mBoard[pos.x + 1][pos.y + 1] |= 32; // place to the left of south-eastern neighbour (32 = vert wall to the left, second segment)

      }
   }

   bool MB6_Board::ComputeMinPath(const qcore::PlayerId& id)
   {
      if (IsInEnemyBase(mPlayerPos[id], id))
      {
         mMinPathLen[id] = 0;
         return true;
      }

      // define a structure for saving information for a tile when it is reached:
      typedef struct
      {
         qcore::Position tilePos = UNDEF_POS;
         qcore::Position prevTilePos = UNDEF_POS;
         uint8_t pathLen = 0; // path length from player to current tile
      }TileInfo_t;

      // save info for each tile to this array as the tiles are being reached
      TileInfo_t visitedTiles[qcore::BOARD_SIZE][qcore::BOARD_SIZE]; // initialized with the values from type definition

      // queue for tile information
      std::queue<TileInfo_t> q;

      // start with player's position (no previous tile and path length 0)
      q.push(TileInfo_t{ mPlayerPos[id], UNDEF_POS, 0 });

      // breadth-first-search: (if queue ever gets empty then there is no valid path from player to enemy base)
      while (!q.empty())
      {
         TileInfo_t tile = q.front();
         q.pop();

         auto NotVisited = [&](const qcore::Position& pos) -> bool { return (visitedTiles[pos.x][pos.y].tilePos == UNDEF_POS); };

         if (NotVisited(tile.tilePos))
         {
            // save info about visited tile
            visitedTiles[tile.tilePos.x][tile.tilePos.y] = tile;

            // check if enemy base was reached
            if(IsInEnemyBase(tile.tilePos, id))
            {
               // found min path
               mMinPathLen[id] = tile.pathLen;

               // make path visible on the board
               #if (DEBUG)
                  qcore::Position pos = tile.tilePos;
                  do
                  {
                     mBoard[pos.x][pos.y] |= (256 * (id + 1));
                     pos = visitedTiles[pos.x][pos.y].prevTilePos;
                  } while (not(pos == mPlayerPos[id]));
               #endif

               return true;
            }

            // go through neighbour tiles and add them to the queue if the min path to them was not found yet
            // 1. up:
            qcore::Position next = tile.tilePos + qcore::Direction::Up;
            if (!HasWallAbove(tile.tilePos) && NotVisited(next))
            {
               q.push(TileInfo_t{ next, tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 2. left:
            next = tile.tilePos + qcore::Direction::Left;
            if (!HasWallToLeft(tile.tilePos) && NotVisited(next))
            {
               q.push(TileInfo_t{ next, tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 3. right:
            next = tile.tilePos + qcore::Direction::Right;
            if (!HasWallToRight(tile.tilePos) && NotVisited(next))
            {
               q.push(TileInfo_t{ next, tile.tilePos, (uint8_t)(tile.pathLen + 1) });
            }

            // 4. down:
            next = tile.tilePos + qcore::Direction::Down;
            if (!HasWallBelow(tile.tilePos) && NotVisited(next))
            {
               q.push(TileInfo_t{ next, tile.tilePos, (uint8_t)(tile.pathLen + 1) });
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

   bool MB6_Board::HasWallAbove(const qcore::Position& pos) const
   {
      return (mBoard[pos.x][pos.y] & (1 | 2));
   }

   bool MB6_Board::HasWallBelow(const qcore::Position& pos) const
   {
      return (mBoard[pos.x][pos.y] & (4 | 8));
   }

   bool MB6_Board::HasWallToLeft(const qcore::Position& pos) const
   {
      return (mBoard[pos.x][pos.y] & (16 | 32));
   }

   bool MB6_Board::HasWallToRight(const qcore::Position& pos) const
   {
      return (mBoard[pos.x][pos.y] & (64 | 128));
   }

   bool MB6_Board::IsInEnemyBase(const qcore::Position& pos, const qcore::PlayerId& id) const
   {
      return ((id == mMyID && pos.x == 0) || // row #0 is enemy base for me
                (id != mMyID && pos.x == 8)); // row #8 is enemy base for opponent
   }

   #if (DEBUG)
   void MB6_Logger::LogBoard(MB6_Board& board, const qcore::PlayerId myID)
   #else
   void MB6_Logger::LogBoard(const MB6_Board& board, const qcore::PlayerId myID)
   #endif
   {
      char map[qcore::BOARD_MAP_SIZE][qcore::BOARD_MAP_SIZE] = { 0 };

      // add walls to map
      for (int8_t i = 0; i < qcore::BOARD_SIZE; i++)
      {
         for (int8_t j = 0; j < qcore::BOARD_SIZE; j++)
         {
            // convert board coords to map coords
            qcore::Position mapPos = BoardToMapPosition(qcore::Position(i, j));

            // start with an empty tile or a path tile
            #if (DEBUG)
               if (board.mBoard[i][j] & 256 && board.mBoard[i][j] & 512) // tile is on the path of both players
               {
                  map[mapPos.x][mapPos.y] = 'x';
               }
               else if (board.mBoard[i][j] & 256) // tile is on player 0's path
               {
                  map[mapPos.x][mapPos.y] = '*';
               }
               else if (board.mBoard[i][j] & 512) // tile is on player 1's path
               {
                  map[mapPos.x][mapPos.y] = '+';
               }
               else // empty tile
               {
                  map[mapPos.x][mapPos.y] = ' ';
               }
            #else
               map[mapPos.x][mapPos.y] = ' ';
            #endif

            // add horizontal wall below if applicable
            if (i < qcore::BOARD_SIZE - 1)
            {
               if (board.mBoard[i][j] & 4)
               {
                  map[mapPos.x + 1][mapPos.y] = '-'; // first segment
               }
               else if (board.mBoard[i][j] & 8)
               {
                  map[mapPos.x + 1][mapPos.y] = '='; // second segment
               }
               else
               {
                  map[mapPos.x + 1][mapPos.y] = ' ';
               }

               //map[mapPos.x + 1][mapPos.y] = (board.mBoard[i][j] & (4 | 8) ? '=' : ' ');
            }

            // add vertical wall to the right if applicable
            if (j < qcore::BOARD_SIZE - 1)
            {
               if (board.mBoard[i][j] & 64)
               {
                  map[mapPos.x][mapPos.y + 1] = '!'; // first segment
               }
               else if (board.mBoard[i][j] & 128)
               {
                  map[mapPos.x][mapPos.y + 1] = '|'; // second segment
               }
               else
               {
                  map[mapPos.x][mapPos.y + 1] = ' ';
               }
               // map[mapPos.x][mapPos.y + 1] = (board.mBoard[i][j] & (64 | 128) ? '|' : ' ');
            }

            // add a dot at the intersection of vertical and horizontal wall lines
            if ((i < qcore::BOARD_SIZE - 1) && (j < qcore::BOARD_SIZE - 1))
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

         for (uint8_t j = 0; j < qcore::BOARD_MAP_SIZE; j++)
         {            
            if ((j % 2) == 0) // check to skip vertical walls and dots
            {
               if (map[i][j] != '0' && map[i][j] != '1') // check to skip tiles with players
               {
                  #if (DEBUG)
                     if ((map[i][j] == '+') || (map[i][j] == '*') || (map[i][j] == 'x')) 
                     {
                        // path tiles
                        mBuff[bIndex++] = map[i][j];
                        mBuff[bIndex++] = ' ';
                     }
                     else
                     {
                        // duplicate free tiles and horiz wall tiles
                        mBuff[bIndex++] = map[i][j];
                        mBuff[bIndex++] = map[i][j];
                     }
                  #else
                     // duplicate free tiles and horiz wall tiles
                     mBuff[bIndex++] = map[i][j];
                     mBuff[bIndex++] = map[i][j];
                  #endif
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

      #if (DEBUG)
      // remove path info from the board
      for (int8_t i = 0; i < qcore::BOARD_SIZE; i++)
      {
         for (int8_t j = 0; j < qcore::BOARD_SIZE; j++)
         {
            board.mBoard[i][j] &= (~256);
            board.mBoard[i][j] &= (~512);
         }
      }
      #endif
   }

   qcore::Position MB6_Logger::BoardToMapPosition(const qcore::Position& pos) const
   {
      return pos * 2;
   }

   void MB6_Logger::ClearBuff(void)
   {
      memset(mBuff, 0, sizeof(mBuff));
   }

   static qcore::Position CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      return (ori == qcore::Orientation::Vertical ? pos - 1_y : pos - 1_x);
   }

   static qcore::Position CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori)
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
