#ifndef H_PLUGIN_MB6_PLAYER
#define H_PLUGIN_MB6_PLAYER

#include "Player.h"
#include "QcoreUtil.h"

#define DEBUG                  true
#define LOGGED_LINE_MAX_LEN    120

namespace qplugin
{
   class MB6_Player : public qcore::Player
   {
      public:
         /** Construction */
         MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

         /** Defines player's behavior. */
         void doNextMove() override;

         uint16_t turn;
   };

   class MB6_Board
   {
      friend class MB6_Logger;
      
      public:
         static qcore::PlayerId mMyID; // 0 (if I am first to move) or 1

         MB6_Board(){};
         void UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos);
         void PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori);
         bool ComputeMinPath(const qcore::PlayerId& id);
         uint8_t GetMinPath(const qcore::PlayerId& id) const;
         bool HasWallAbove(const qcore::Position& pos) const;
         bool HasWallBelow(const qcore::Position& pos) const;
         bool HasWallToLeft(const qcore::Position& pos) const;
         bool HasWallToRight(const qcore::Position& pos) const;
         bool IsInEnemyBase(const qcore::Position& pos, const qcore::PlayerId& id) const; // true if given pos is part of the enemy base for player with given id

      private:
         uint8_t mBoard[qcore::BOARD_SIZE][qcore::BOARD_SIZE] = 
         {
            // Horizontal wall: above = 1, below = 2; Vertical wall: to the left = 4, to the right = 8
            { 5, 1, 1, 1, 1, 1, 1, 1, 9 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 4, 0, 0, 0, 0, 0, 0, 0, 8 },
            { 6, 2, 2, 2, 2, 2, 2, 2, 10}
         };
         
         qcore::Position mPlayerPos[2]; // index is playerID, e.g. mPlayerPos[mMyID] = my pos
         uint8_t mMinPathLen[2]; // index is playerID, e.g. mMinPathLen[mMyID] = my min path length
   };

   class MB6_Logger
   {
      public:
         #if (DEBUG)
         void LogBoard(MB6_Board& board, const qcore::PlayerId myID);
         #else
         void LogBoard(const MB6_Board& board, const qcore::PlayerId myID);
         #endif

      private:
         qcore::Position BoardToMapPosition(const qcore::Position& pos) const;
         void ClearBuff(void);

         char mBuff[LOGGED_LINE_MAX_LEN] = { 0 };
   };

   
} // end namespace
#endif // H_PLUGIN_MB6_PLAYER
