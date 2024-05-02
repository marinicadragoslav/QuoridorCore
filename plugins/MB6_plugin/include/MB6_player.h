#ifndef H_PLUGIN_MB6_PLAYER
#define H_PLUGIN_MB6_PLAYER

#include "Player.h"

namespace qplugin
{
   class MB6_Player : public qcore::Player
   {
      public:
         MB6_Player(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);
         void doNextMove() override; // defines player's behaviour
         qcore::Position CoreToPluginWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
         qcore::Position PluginToCoreWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
         qcore::Position CoreAbsToRelWallPos(const qcore::Position& pos, const qcore::Orientation& ori);
         qcore::Position CoreRelToAbsWallPos(const qcore::Position& pos, const qcore::Orientation& ori);


         uint16_t mTurnCount;
   };

} // end namespace

#endif // H_PLUGIN_MB6_PLAYER
