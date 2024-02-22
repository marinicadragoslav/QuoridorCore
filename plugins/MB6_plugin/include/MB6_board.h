#ifndef H_PLUGIN_MB6_BOARD
#define H_PLUGIN_MB6_BOARD

#include "Player.h"

namespace qplugin
{
   typedef enum
   {
      WALL_N1  = (1 << 0),
      WALL_N2  = (1 << 1),
      WALL_S1  = (1 << 2),
      WALL_S2  = (1 << 3),
      WALL_W1  = (1 << 4),
      WALL_W2  = (1 << 5),
      WALL_E1  = (1 << 6),
      WALL_E2  = (1 << 7),
      WALL_N   = (WALL_N1 | WALL_N2),
      WALL_S   = (WALL_S1 | WALL_S2),
      WALL_W   = (WALL_W1 | WALL_W2),
      WALL_E   = (WALL_E1 | WALL_E2),
      WALL_NW  = (WALL_N | WALL_W),
      WALL_NE  = (WALL_N | WALL_E),
      WALL_SW  = (WALL_S | WALL_W),
      WALL_SE  = (WALL_S | WALL_E),
#if(MB6_DEBUG)
      MARKER_MY_PATH = (1 << 8),
      MARKER_OPP_PATH = (1 << 9)
#endif
   }Wall_t;

   typedef struct
   {
      qcore::Position start = UNDEF_POS;
      qcore::Position end = UNDEF_POS;
      uint8_t pathLen = 0; // total path length from player to End
   }Step_t;

   class MB6_Board
   {
      friend class MB6_Logger;
      
      public:
         MB6_Board(){};
         void UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos);
         void UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls);
         void PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori);
         uint8_t GetMinPath(const qcore::PlayerId& id);
         bool HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const;
         bool IsWinningPos(const qcore::Position& pos, const qcore::PlayerId& id) const;
#if(MB6_DEBUG)
         void MarkMinPath(const Step_t steps[][qcore::BOARD_SIZE], const qcore::Position& end, const qcore::PlayerId& id);
#endif

      private:
         qcore::Position mPlayerPos[2]; // index is player's ID
         uint8_t mWallsLeft[2]; // index is player's ID
#if (MB6_DEBUG) 
         uint16_t mBoard[qcore::BOARD_SIZE][qcore::BOARD_SIZE] =
#else 
         uint8_t mBoard[qcore::BOARD_SIZE][qcore::BOARD_SIZE] = 
#endif
         {
            { WALL_NW,  WALL_N,  WALL_N,  WALL_N,  WALL_N,  WALL_N,  WALL_N,  WALL_N,  WALL_NE },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_W,   0,       0,       0,       0,       0,       0,       0,       WALL_E  },
            { WALL_SW,  WALL_S,  WALL_S,  WALL_S,  WALL_S,  WALL_S,  WALL_S,  WALL_S,  WALL_SE },
         };
   };// end class MB6_Board

} // end namespace

#endif // H_PLUGIN_MB6_BOARD
