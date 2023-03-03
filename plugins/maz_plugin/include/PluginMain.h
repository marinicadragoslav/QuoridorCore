#ifndef Header_qcore_DummyPlayer
#define Header_qcore_DummyPlayer

#include  <future>

#include "ABotV2Analyser.h"

namespace qplugin
{
   class ABotV2 : public qcore::Player
   {
   public:

      /** Construction */
      ABotV2(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;

   private:
      ABotV2Analyser m_analyser;

      std::future<void> m_dummyFuture;
   };
}

#endif // Header_qcore_DummyPlayer
