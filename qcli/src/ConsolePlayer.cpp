#include "ConsolePlayer.h"

namespace qcli
{
   ConsolePlayer::ConsolePlayer(uint8_t id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** Defines player's behavior */
   void ConsolePlayer::doNextMove()
   {
   }

} // namespace qcli
