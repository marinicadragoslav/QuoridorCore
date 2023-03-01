#ifndef Header_qplugin_drma_player
#define Header_qplugin_drma_player

#include "Player.h"
#include <stdbool.h>

#define BOARD_SZ                       (9)
#define MINIMAX_DEPTH                  (3)
#define MINIMAX_TIMEOUT_MS             (4500)
#define SHOW_MIN_PATH_ON_LOGGED_BOARD  (false)
#define POS_INFINITY                   (+0xFFFFFF)     // some large positive value
#define NEG_INFINITY                   (-0xFFFFFF)     // some large negative value
#define CAP_POS_SCORE                  (+0xFFFFF0)     // valid minimax score < CAP_POS_SCORE < POS_INFINITY
#define CAP_NEG_SCORE                  (-0xFFFFF0)     // valid minimax score > CAP_NEG_SCORE > NEG_INFINITY
#define ERROR_NO_PATH                  (-0xFFFFFFF)    // must be outside [NEG_INFINITY, POS_INFINITY] range
#define RUN_TESTS                      (true)

namespace qplugin_drma
{
   class drmaPlayer : public qcore::Player
   {
   public:

      /** Construction */
      drmaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;
   };
   
} // end namespace
#endif // Header_qplugin_drma_player
