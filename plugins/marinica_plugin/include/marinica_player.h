#ifndef Header_qplugin_marinica_player
#define Header_qplugin_marinica_player

#include "Player.h"

namespace qplugin
{
   class MarinicaPlayer : public qcore::Player
   {
   public:

      /** Construction */
      MarinicaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;
   };
}

#endif // Header_qplugin_marinica_player
