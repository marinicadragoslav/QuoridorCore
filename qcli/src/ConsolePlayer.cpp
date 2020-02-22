#include "ConsolePlayer.h"

#include <iostream>

namespace qcli
{
   ConsolePlayer::ConsolePlayer(uint8_t id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** Defines player's behavior */
   void ConsolePlayer::doNextMove()
   {
      std::cout << "Player " << (int)getId() << "'s turn [" << (int) getWallsLeft()
         << " wall" << (getWallsLeft() == 1 ? "" : "s") << " left]\n>";
   }

} // namespace qcli
