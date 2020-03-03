#include "ICPlayer.h"
#include "QcoreUtil.h"

#include <queue>
#include <sstream>
#include <iomanip>
#include <chrono>

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace
{
   /** Log domain */
   const char * const DOM = "qplugin::IC";
}

namespace util
{
   void PrintAsciiGameBoard(const qcore::BoardMap& map)
   {
      std::stringstream ss;
      ss << "\n";

      for (int i = 0; i < qcore::BOARD_SIZE; ++i)
      {
         ss << std::setfill(' ') << std::setw(6) << i;
      }

      ss << "\n   0 \u2554";

      for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
      {
         ss << "\u2550";
      }

      ss << "\u2557\n";

      for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
      {
         if (i & 1)
         {
            ss << std::setfill(' ') << std::setw(4) << (i / 2) + 1 << " \u2551 ";
         }
         else
         {
            ss << "     \u2551 ";
         }

         for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
         {
            switch (map(i, j))
            {
               case 0:
                  ss << "   ";
                  break;
               case qcore::BoardMap::VertivalWall:
                  ss << " \u2502 ";
                  break;
               case qcore::BoardMap::HorizontalWall:
                  ss << "\u2500\u2500\u2500";
                  break;
               case qcore::BoardMap::Pawn0:
                  ss << " 0 ";
                  break;
               case qcore::BoardMap::Pawn1:
                  ss << " 1 ";
                  break;
               case qcore::BoardMap::Pawn2:
                  ss << " 2 ";
                  break;
               case qcore::BoardMap::Pawn3:
                  ss << " 3 ";
                  break;
               default:
                  ss << " " << map(i, j) << " ";
                  break;
            }
         }

         ss << " \u2551\n";
      }

      ss << "     \u255A";

      for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
      {
         ss << "\u2550";
      }

      ss << "\u255D\n";

      LOG_TRACE(DOM) << ss.str();
   }

   bool wallValid(const qcore::Player& player, const qcore::BoardMap& map, const qcore::WallState& wall)
   {
      if (not player.isValid(wall))
         return false;

      // Check if it is not intersecting other wall
      if (wall.orientation == qcore::Orientation::Vertical)
      {
         qcore::Position p = wall.position * 2 - 1_y;

         if (map(p) or map(p + 1_x) != qcore::BoardMap::MidWall or map(p + 2_x))
         {
            return false;
         }
      }
      else
      {
         qcore::Position p = wall.position * 2 - 1_x;

         if (map(p) or map(p + 1_y) != qcore::BoardMap::MidWall or map(p + 2_y))
         {
            return false;
         }
      }

      return true;
   }
}

namespace qplugin
{
   BState::BState(const qcore::Player& player, bool opw) :
      level( 0 )
   {
      player.getBoardState()->createBoardMap(baseMap, player.getId());
      myPos = player.getBoardState()->getPlayers(player.getId()).at(player.getId()).position;
      opPos = player.getBoardState()->getPlayers(player.getId()).at(player.getId() ^ 1).position;

      compute(baseMap, myPos, true);
      compute(player, opw);
   }

   BState::BState(const BState& bs, const qcore::WallState& wall, const qcore::Player& player, bool opw, int level) :
      level( level == -1 ? bs.level + 1 : level ),
      baseMap( bs.baseMap ),
      myPos( bs.myPos ),
      opPos( bs.opPos ),
      wall( wall ),
      initWall( bs.level == 0 ? wall : bs.initWall )
   {
      if (wall.orientation == qcore::Orientation::Vertical)
      {
         qcore::Position p = wall.position * 2 - 1_y;
         baseMap(p) = baseMap(p + 1_x) = baseMap(p + 2_x) = qcore::BoardMap::VertivalWall;
      }
      else
      {
         qcore::Position p = wall.position * 2 - 1_x;
         baseMap(p) = baseMap(p + 1_y) = baseMap(p + 2_y) = qcore::BoardMap::HorizontalWall;
      }

      compute(player, opw);
   }

   bool BState::operator<(const BState& bs)
   {
      if (score != bs.score)
      {
         return score < bs.score;
      }

      if (level != bs.level)
      {
         return level > bs.level;
      }

      if (opPathNo != bs.opPathNo)
      {
         return opPathNo > bs.opPathNo;
      }

      return myPathNo < bs.myPathNo;
   }

   void BState::compute(const qcore::Player& player, bool opw)
   {
      if (not opw or not player.getBoardState()->getPlayers(0).at(player.getId() ^ 1).wallsLeft)
      {
         myPathLen = compute(baseMap, myPos, true);
         opPathLen = compute(baseMap, opPos, false);
         score = opPathLen - myPathLen - level;
      }
      else
      {
         qcore::BoardMap refMap;
         score = INT32_MAX;
         myPathLen = -1;
         opPathLen = -1;

         for (int i = 0; i < qcore::BOARD_SIZE - 1; ++i)
         {
            for (int j = 0; j < qcore::BOARD_SIZE - 1; ++j)
            {
               qcore::WallState ws1 { qcore::Position(i, j), qcore::Orientation::Vertical };
               qcore::WallState ws2 { qcore::Position(i, j), qcore::Orientation::Horizontal };

               if (util::wallValid(player, baseMap, ws1))
               {
                  auto b1 = std::make_shared<BState>(*this, ws1, player, false, level);

                  if (b1->myPathLen > 0 and b1->opPathLen > 0 and *b1 < *this)
                  {
                     myPathLen = b1->myPathLen;
                     opPathLen = b1->opPathLen;
                     myPathNo = b1->myPathNo;
                     opPathNo = b1->opPathNo;
                     score = b1->opPathLen - b1->myPathLen - level;
                     refMap = b1->baseMap;
                  }
               }

               if (util::wallValid(player, baseMap, ws2))
               {
                  auto b2 = std::make_shared<BState>(*this, ws2, player, false, level);

                  if (b2->myPathLen > 0 and b2->opPathLen > 0 and *b2 < *this)
                  {
                     myPathLen = b2->myPathLen;
                     opPathLen = b2->opPathLen;
                     myPathNo = b2->myPathNo;
                     opPathNo = b2->opPathNo;
                     score = b2->opPathLen - b2->myPathLen - level;
                     refMap = b2->baseMap;
                  }
               }
            }
         }

         if (level == 1)
            baseMap = refMap;
      }
   }

   int BState::compute(const qcore::BoardMap& baseMap, const qcore::Position& startPos, bool me)
   {
      qcore::Position myPos = startPos * 2;
      std::list<qcore::Position> pList;
      std::list<int> steps;
      qcore::BoardMap map = baseMap;
      int result = -1;
      int &pathNo = me ? myPathNo : opPathNo;
      pathNo = -1;

      for (uint8_t i = 0; i < qcore::BOARD_MAP_SIZE; i += 2)
      {
         qcore::Position p = me ? qcore::Position(0, i) : qcore::Position(qcore::BOARD_MAP_SIZE - 1, i);
         map(p) = qcore::BoardMap::Invalid;
         pList.push_back(p);
         steps.push_back(1);
      }

      auto checkPos = [&](const qcore::Position& p, qcore::Direction d, int s) -> bool
      {
         if (myPos == p)
         {
            if (result == -1)
            {
               result = s;
               pathNo = 1;

               if ( me )
               {
                  direction = d;
               }
            }
            else if (result == s)
            {
               ++pathNo;
            }
            else
            {
               return true;
            }
         }

         if(map(p) < qcore::BoardMap::HorizontalWall)
         {
            map(p) = qcore::BoardMap::Invalid;
            pList.emplace_back(p);
            steps.push_back(s + 1);
         }

         return false;
      };

      while (not pList.empty())
      {
         auto p = pList.front();
         auto s = steps.front();
         pList.pop_front();
         steps.pop_front();

         if ((map(p + 1_x) == 0 and checkPos(p + 2_x, qcore::Direction::Up, s)) or
            (map(p - 1_x) == 0 and checkPos(p - 2_x, qcore::Direction::Down, s)) or
            (map(p + 1_y) == 0 and checkPos(p + 2_y, qcore::Direction::Left, s)) or
            (map(p - 1_y) == 0 and checkPos(p - 2_y, qcore::Direction::Right, s)))
         {
            break;
         }
      }

      return result;
   }

   ICPlayer::ICPlayer(uint8_t id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game),
      mMoves(0)
   {
   }

   void ICPlayer::doNextMove()
   {
      LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking ..";

      auto time = std::chrono::steady_clock::now();
      auto best = std::make_shared<BState>(*this, true);

      if (mMoves > 2 and getWallsLeft())
      {
         std::priority_queue<std::shared_ptr<BState>, std::vector<std::shared_ptr<BState>>, BStateOrder> queue;
         queue.emplace(best);

         while (not queue.empty())
         {
            auto bs = queue.top();
            queue.pop();

            if (bs->level > getWallsLeft())
            {
               break;
            }

            LOG_TRACE(DOM) << "Finishing in " << bs->myPathLen << " VS " << bs->opPathLen << " moves [lv: " << bs->level << ", sc: " << bs->score << "]";
            LOG_TRACE(DOM) << "                                             >> score [" << bs->score << "] level [" << bs->level << "] wall ["
               << (int) bs->wall.position.x << ", " << (int) bs->wall.position.y << ", " << (bs->wall.orientation == qcore::Orientation::Vertical ? "V" : "H" ) << "]";

            util::PrintAsciiGameBoard(bs->baseMap);

            if (bs->score > best->score and (best->score <= 5 or getWallsLeft() > 2))
            {
               best = bs;
            }

            if (bs->level >= 4)
            {
               break;
            }

            if (std::chrono::steady_clock::now() - time > 10s)
            {
               LOG_TRACE(DOM) << "TIMEOUT";
               break;
            }

            for (int i = 0; i < qcore::BOARD_SIZE - 1; ++i)
            {
               for (int j = 0; j < qcore::BOARD_SIZE - 1; ++j)
               {
                  qcore::WallState ws1 { qcore::Position(i, j), qcore::Orientation::Vertical };
                  qcore::WallState ws2 { qcore::Position(i, j), qcore::Orientation::Horizontal };

                  if (util::wallValid(*this, bs->baseMap, ws1))
                  {
                     auto b1 = std::make_shared<BState>(*bs, ws1, *this, bs->level == 0);

                     if (b1->myPathLen > 0 and b1->opPathLen > 0 and (bs->level == 0 or b1->score > bs->score - 3))
                     {
                        queue.push(b1);
                     }
                  }

                  if (util::wallValid(*this, bs->baseMap, ws2))
                  {
                     auto b2 = std::make_shared<BState>(*bs, ws2, *this, bs->level == 0);

                     if (b2->myPathLen > 0 and b2->opPathLen > 0 and (bs->level == 0 or b2->score > bs->score - 3))
                     {
                        queue.push(b2);
                     }
                  }
               }
            }
         }

         LOG_TRACE(DOM) << "queue size" << queue.size();
      }

      LOG_TRACE(DOM) << "BEST   >> Score [" << best->score << "] level [" << best->level << "] path [" << best->myPathLen << " VS " << best->opPathLen
         << "] pathNo [" << best->myPathNo << " VS " << best->opPathNo << "] wall [" << (int) best->initWall.position.x << ", " << (int) best->initWall.position.y
         << ", " << (best->initWall.orientation == qcore::Orientation::Vertical ? "V" : "H" ) << "]";

      if (best->level == 0)
      {
         move(best->direction);

         if (best->score < -2)
         {
            LOG_INFO(DOM) << "It seems I've already lost. Good game!";
         }
      }
      else
      {
         placeWall(best->initWall);
      }

      ++mMoves;
   }

} // namespace qplugin
