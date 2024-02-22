#ifndef H_PLUGIN_MB6_PLAYER
#define H_PLUGIN_MB6_PLAYER

#include "Player.h"

#define MB6_DEBUG             (false)
#define UNDEF_POS             (qcore::Position{-0xF, -0xF})
#define UNDEF_ID              (0xF)
#define INFINITE_LEN          (0xFF)

namespace qplugin
{
   class MB6_Player : public qcore::Player
   {
      public:
         MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);
         void doNextMove() override; // defines player's behaviour
         qcore::Position CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
         qcore::Position CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori);

         static qcore::PlayerId _mMyId;
         static qcore::PlayerId _mOppId;
         uint16_t mTurnCount;
   };

} // end namespace

#endif // H_PLUGIN_MB6_PLAYER
