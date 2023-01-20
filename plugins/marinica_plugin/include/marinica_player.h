#ifndef Header_qcore_marinica_player
#define Header_qcore_marinica_player

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

#endif // Header_qcore_marinica_player
