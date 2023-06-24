#ifndef H_PLUGIN_MB6_PLAYER
#define H_PLUGIN_MB6_PLAYER

#include "Player.h"

#define DEBUG                 true
#define UNDEF_POS             qcore::Position{-0xF, -0xF}
#define LOGGED_LINE_MAX_LEN   120


namespace qplugin
{
   class MB6_Player : public qcore::Player
   {
      public:
         /** Construction */
         MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

         /** Defines player's behavior. */
         void doNextMove() override;

         uint16_t mTurnCount;
   };

   class MB6_Board
   {
      friend class MB6_Logger;
      
      public:
         typedef enum
         {
            WALL_NORTH_1 = (1 << 0),
            WALL_NORTH_2 = (1 << 1),
            WALL_SOUTH_1 = (1 << 2),
            WALL_SOUTH_2 = (1 << 3),
            WALL_WEST_1 = (1 << 4),
            WALL_WEST_2 = (1 << 5),
            WALL_EAST_1 = (1 << 6),
            WALL_EAST_2 = (1 << 7),
            WALL_NORTH = (WALL_NORTH_1 | WALL_NORTH_2),
            WALL_SOUTH = (WALL_SOUTH_1 | WALL_SOUTH_2),
            WALL_WEST = (WALL_WEST_1 | WALL_WEST_2),
            WALL_EAST = (WALL_EAST_1 | WALL_EAST_2),
            WALL_NORTHWEST = (WALL_NORTH | WALL_WEST),
            WALL_NORTHEAST = (WALL_NORTH | WALL_EAST),
            WALL_SOUTHWEST = (WALL_SOUTH | WALL_WEST),
            WALL_SOUTHEAST = (WALL_SOUTH | WALL_EAST),
            #if(DEBUG)
            MARKER_MY_PATH = (1 << 8),
            MARKER_OPP_PATH = (1 << 9)
            #endif
         }TileElements_t;

         // When computing min path: save tile info to a matrix of this type
         typedef struct
         {
            qcore::Position tilePos = UNDEF_POS;
            qcore::Position prevTilePos = UNDEF_POS;
            uint8_t pathLen = 0; // path length from player to current tile
         }TileInfo_t;

         static qcore::PlayerId mMyID; // 0 (if I am first to move) or 1

         MB6_Board(){};
         void UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos);
         void UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls);
         void PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori);
         bool ComputeMinPath(const qcore::PlayerId& id);
         uint8_t GetMinPath(const qcore::PlayerId& id) const;
         bool HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const;
         bool IsPosInPlayersEnemyBase(const qcore::Position& pos, const qcore::PlayerId& id) const;
         #if(DEBUG)
         void MarkMinPathOnBoard(const TileInfo_t visitedTiles[][qcore::BOARD_SIZE], const qcore::Position& startPos, const qcore::PlayerId& id);
         #endif

      private:
         #if (DEBUG) 
         uint16_t 
         #else 
         uint8_t 
         #endif 
         mBoard[qcore::BOARD_SIZE][qcore::BOARD_SIZE] = 
         {
            { WALL_NORTHWEST,  WALL_NORTH,  WALL_NORTH,  WALL_NORTH,  WALL_NORTH,  WALL_NORTH,  WALL_NORTH,  WALL_NORTH,  WALL_NORTHEAST },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_WEST,       0,           0,           0,           0,           0,           0,           0,           WALL_EAST      },
            { WALL_SOUTHWEST,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTH,  WALL_SOUTHEAST },
         };
         
         qcore::Position mPlayerPos[2]; // index is playerID, e.g. mPlayerPos[mMyID] = my pos
         uint8_t mMinPathLen[2]; // index is playerID, e.g. mMinPathLen[mMyID] = my min path length
         uint8_t mWallsLeft[2]; // index is playerID, e.g. mWallsLeft[mMyID] = walls left for me
   };   
} // end namespace

#endif // H_PLUGIN_MB6_PLAYER
