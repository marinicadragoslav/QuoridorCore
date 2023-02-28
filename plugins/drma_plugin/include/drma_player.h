#ifndef Header_qplugin_drma_player
#define Header_qplugin_drma_player

#include "Player.h"
#include <stdbool.h>

namespace qplugin_drma
{
   #define RUN_TESTS       true
   #define MINIMAX_LEVEL   3

   class drmaPlayer : public qcore::Player
   {
   public:

      /** Construction */
      drmaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;
   };
}

#endif // Header_qplugin_drma_player
