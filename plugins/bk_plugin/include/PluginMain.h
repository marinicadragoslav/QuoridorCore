#ifndef Header_qcore_DummyPlayer
#define Header_qcore_DummyPlayer

#include "mcts/mcts.h"
#include "mcts_impl.h"

namespace qplugin
{
   class BK_Plugin : public qcore::Player
   {
   public:

      /** Construction */
      BK_Plugin(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;

   private:
      MCTS_tree *game_tree = nullptr;
      Quoridor_state *state = nullptr;
   };
}

#endif // Header_qcore_DummyPlayer
