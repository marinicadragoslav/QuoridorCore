#ifndef H_PLUGIN_MB6_PLAYER
#define H_PLUGIN_MB6_PLAYER

#include "Player.h"
#include "QcoreUtil.h"


#define LOGGED_LINE_MAX_LEN            (120)

namespace qplugin
{
   class MB6_Player : public qcore::Player
   {
      public:

         /** Construction */
         MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

         /** Defines player's behavior. In this particular case, it's a really dummy one */
         void doNextMove() override;

      private:
      
         // Convert the absolute position of a wall to relative position (flip around the center point on both axis)
         qcore::Position CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation);

         // Convert position of a wall from core coords to plugin coords
         qcore::Position CoreToPluginWallPos(qcore::Position corePos, qcore::Orientation orientation);

         // Convert position of a player from core coords to plugin coords
         qcore::Position CoreToPluginPlayerPos(qcore::Position corePos);

         uint16_t turn = 0;
   };

   class MB6_Logger;

   class MB6_Board
   {
      friend class MB6_Logger;
      
      public:
         // make this static because: once set, it doesn't change & no need to duplicate it when creating board copies
         static uint8_t mEnemyBaseRow[2]; // index is playerID, e.g. mEnemyBaseRow[myID] = enemy base row for me

         void UpdatePos(qcore::PlayerId id, qcore::Position pos);
         void PlaceWall(qcore::Position pos, qcore::Orientation orientation);
         bool ComputeMinPath(qcore::PlayerId id);
         uint8_t GetMinPath(qcore::PlayerId id);
         bool HasWallAbove(qcore::Position);
         bool HasWallBelow(qcore::Position);
         bool HasWallToLeft(qcore::Position);
         bool HasWallToRight(qcore::Position);

      private:
         uint8_t mBoard[qcore::BOARD_SIZE + 1][qcore::BOARD_SIZE + 1] = 
         {
            {  3,1,1,1,1,1,1,1,1,3  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  2,0,0,0,0,0,0,0,0,2  },
            {  3,1,1,1,1,1,1,1,1,3  }
         };

         qcore::Position mPlayerPos[2]; // index is playerID, e.g. mPlayerPos[myID] = my pos
         uint8_t mMinPathLen[2];        // index is playerID, e.g. mMinPathLen[myID] = my min path length
   };

   class MB6_Logger
   {
      public:
         void LogBoard(MB6_Board board, qcore::PlayerId myID);

      private:
         qcore::Position BoardToMapPosition(qcore::Position);
         void ClearBuff(void);

         char mBuff[LOGGED_LINE_MAX_LEN] = { 0 };
   };

   
} // end namespace
#endif // H_PLUGIN_MB6_PLAYER
