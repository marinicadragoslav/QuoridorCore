#include "DummyPlayer.h"
#include "QcoreUtil.h"

#include <thread>

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace qplugin
{
   /** Log domain */
   const char * const DOM = "qplugin::PL";

   DummyPlayer::DummyPlayer(uint8_t id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   /** Defines player's behavior. In this particular case, it's a really dummy one */
   void DummyPlayer::doNextMove()
   {
      LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking..";

      // Simulate more thinking
      std::this_thread::sleep_for(1000ms);

      qcore::Position myPos = getPosition() * 2;
      qcore::BoardMap map;
      std::list<qcore::Position> pos;

      getBoardState()->createBoardMap(map, getId());

      for (uint8_t i = 0; i < qcore::BOARD_MAP_SIZE; i += 2)
      {
         qcore::Position p(0, i);
         map(p) = qcore::BoardMap::Invalid;
         pos.push_back(p);
      }

      auto checkPos = [&](const qcore::Position& p, qcore::Direction dir) -> bool
      {
         if (myPos == p)
         {
            move(dir);
            return true;
         }

         if(map(p) < qcore::BoardMap::HorizontalWall)
         {
            map(p) = qcore::BoardMap::Invalid;
            pos.emplace_back(p);
         }

         return false;
      };

      while (not pos.empty())
      {
         auto p = pos.front();
         pos.pop_front();

         if ((map(p + 1_x) == 0 and checkPos(p + 2_x, qcore::Direction::Up)) or
            (map(p - 1_x) == 0 and checkPos(p - 2_x, qcore::Direction::Down)) or
            (map(p + 1_y) == 0 and checkPos(p + 2_y, qcore::Direction::Left)) or
            (map(p - 1_y) == 0 and checkPos(p - 2_y, qcore::Direction::Right)))
         {
            return;
         }
      }

      LOG_WARN(DOM) << "Something went wrong. Making a random move.";
      move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }

} // namespace qplugin
