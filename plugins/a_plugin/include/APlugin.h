#ifndef Header_qcore_DummyPlayer
#define Header_qcore_DummyPlayer

#include "Player.h"

namespace qplugin
{
   class A_Plugin : public qcore::Player
   {
   public:

      /** Construction */
	   A_Plugin(uint8_t id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. */
      void doNextMove() override;
   };
}

#endif // Header_qcore_DummyPlayer
