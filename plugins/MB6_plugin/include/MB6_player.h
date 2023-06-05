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
         #if (DEBUG) 
            uint16_t 
         #else 
            uint8_t 
         #endif 
         mBoard[qcore::BOARD_SIZE][qcore::BOARD_SIZE] = 
         {
            // Horizontal wall: Above: first segment = 1, second segment = 2; Below: first segment = 4, second segment = 8;
            // Vertical wall: To the left: first segment = 16, second segment = 32; To the right: first segment = 64, second segment = 128;
            { 17,  1,  1,  1,  1,  1,  1,  1,  65 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 16,  0,  0,  0,  0,  0,  0,  0,  64 },
            { 20,  4,  4,  4,  4,  4,  4,  4,  68 },
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
