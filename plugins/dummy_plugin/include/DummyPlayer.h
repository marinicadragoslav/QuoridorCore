#ifndef Header_qcore_DummyPlayer
#define Header_qcore_DummyPlayer

#include "Player.h"

namespace qplugin
{
   class DummyPlayer : public qcore::Player
   {
   public:

      /** Construction */
      DummyPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;
   };
}

#endif // Header_qcore_DummyPlayer
