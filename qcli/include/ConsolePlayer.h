#ifndef Header_qcore_ConsolePlayer
#define Header_qcore_ConsolePlayer

#include "Player.h"

namespace qcli
{
   class ConsolePlayer : public qcore::Player
   {
   public:

      /** Construction */
      ConsolePlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior */
      void doNextMove() override;
   };
}

#endif // Header_qcore_ConsolePlayer
