#ifndef H_PLUGIN_MB6_BOARD
#define H_PLUGIN_MB6_BOARD

#include "PlayerAction.h"
#include "MB6_timer.h"
#include <vector>

#define DEBUG_BOARD           true

#define UNDEFINED_POSITION    (qcore::Position{-0xF, -0xF})
#define UNDEFINED_PLAYER_ID   0xF
#define INEXISTENT_PATH       0xFF

#define POS_SCORE_LIMIT       (+0xFFFFF0)  // limit for valid positive minimax score (must be < POS_INFINITY)
#define NEG_SCORE_LIMIT       (-0xFFFFF0)  // limit for valid negative minimax score (must be > NEG_INFINITY)
#define POS_INFINITY          (+0xFFFFFF)  // some large positive value
#define NEG_INFINITY          (-0xFFFFFF)  // some large negative value
#define OUT_OF_RANGE          (-0xFFFFFFF) // must be outside [NEG_INFINITY, POS_INFINITY] range
#define IS_SCORE_VALID(score) (NEG_SCORE_LIMIT <= score and score <= POS_SCORE_LIMIT)

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
#if(DEBUG_BOARD)
      MARKER_MY_PATH = (1 << 8),
      MARKER_OPP_PATH = (1 << 9)
#endif
   }Wall_t;

   typedef enum
   {
      ACT_TYPE_MOVE,
      ACT_TYPE_H_WALL,
      ACT_TYPE_V_WALL
   }ActionType_t;

   typedef struct
   {
      ActionType_t actionType;
      qcore::Position position;
   }Action_t;
   

   typedef struct
   {
      qcore::Position start = UNDEFINED_POSITION;
      qcore::Position end = UNDEFINED_POSITION;
      uint8_t pathLen = 0; // total path length from player to end
   }Step_t;

   class MB6_Board
   {
      friend class MB6_Logger;

      public:
         MB6_Board(){};
         void UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos);
         void UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls);
         void DecrementWallsLeft(const qcore::PlayerId& id);
         void PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori);
         uint8_t GetMinPath(const qcore::PlayerId& id);
         bool HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const;
         bool IsWinningPos(const qcore::Position& pos, const qcore::PlayerId& id) const;
         bool IsGameOver() const;
         int StaticEval();
         int Minimax(const qcore::PlayerId& id, const uint8_t level, int alpha, int beta, const MB6_Timer& timer, 
               const bool useTimer, bool& timerHasElapsed);
         std::vector<Action_t> GetPossibleActions(const qcore::PlayerId& id) const;
         Action_t GetBestAction(const uint8_t level);
#if(DEBUG_BOARD)
         void MarkMinPath(const Step_t steps[][qcore::BOARD_SIZE], const qcore::Position& end, const qcore::PlayerId& id);
#endif

         static qcore::PlayerId _mMyId;
         static qcore::PlayerId _mOppId;
         static std::vector<Action_t> _mBestActs;

      private:
         qcore::Position mPlayerPos[2]; // index is player's ID
         uint8_t mWallsLeft[2]; // index is player's ID
#if (DEBUG_BOARD) 
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
