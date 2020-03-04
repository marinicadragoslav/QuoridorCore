#include "ConsolePlayer.h"

namespace qcli
{
   ConsolePlayer::ConsolePlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** Defines player's behavior */
   void ConsolePlayer::doNextMove()
   {
   }

} // namespace qcli
