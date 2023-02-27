#ifndef Header_qplugin_d_dragoslav_player
#define Header_qplugin_d_dragoslav_player

#include "Player.h"
#include <stdbool.h>

#define RUN_TESTS       true
#define MINIMAX_LEVEL   3

namespace qplugin_d
{
   class dragoslavPlayer : public qcore::Player
   {
   public:

      /** Construction */
      dragoslavPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;
   };
}

#endif // Header_qplugin_d_dragoslav_player
