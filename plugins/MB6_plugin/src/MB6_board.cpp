#include "MB6_player.h"
#include "MB6_logger.h"
#include <queue>
#include <unordered_map>

using namespace qcore::literals;
using namespace std::chrono_literals;

namespace qplugin
{
   static const std::vector<qcore::Direction> directions = 
   {
      qcore::Direction::Up,
      qcore::Direction::Left,
      qcore::Direction::Right,
      qcore::Direction::Down
   };

   static const std::unordered_map<qcore::Direction, Wall_t> dirToWallMap = 
   {
      {qcore::Direction::Up,     WALL_N},
      {qcore::Direction::Down,   WALL_S},
      {qcore::Direction::Left,   WALL_W},
      {qcore::Direction::Right,  WALL_E}
   };

   void MB6_Board::UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos)
   {
      mPlayerPos[id] = pos;
   }

   void MB6_Board::UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls)
   {
      mWallsLeft[id] = walls;
   }

   void MB6_Board::PlaceWall(const qcore::Position& pos, const qcore::Orientation& ori)
   {
      qcore::Position south = pos + qcore::Direction::Down;
      qcore::Position east = pos + qcore::Direction::Right;
      qcore::Position southeast = south + qcore::Direction::Right;

      if (ori == qcore::Orientation::Horizontal)
      {
         mBoard[pos.x][pos.y] |= WALL_S1;
         mBoard[east.x][east.y] |= WALL_S2;
         mBoard[south.x][south.y] |= WALL_N1;
         mBoard[southeast.x][southeast.y] |= WALL_N2;
      }
      else
      {
         mBoard[pos.x][pos.y] |= WALL_E1;
         mBoard[south.x][south.y] |= WALL_E2;
         mBoard[east.x][east.y] |= WALL_W1;
         mBoard[southeast.x][southeast.y] |= WALL_W2;
      }
   }

   uint8_t MB6_Board::GetMinPath(const qcore::PlayerId& id)
   {
      Step_t steps[qcore::BOARD_SIZE][qcore::BOARD_SIZE]; // initialized at type definition
      std::queue<Step_t> q;

      auto NotVisited = [&](qcore::Position pos) -> bool
      {
         return (steps[pos.x][pos.y].end == UNDEF_POS);
      };

      if (IsWinningPos(mPlayerPos[id], id))
      {
         return 0;
      }

      q.push(Step_t{UNDEF_POS, mPlayerPos[id], 0}); // initial step from nowhere to player's position, path len = 0

      // breadth-first-search: (if queue ever gets empty then there is no valid path from player to enemy base)
      while (!q.empty())
      {
         Step_t step = q.front();
         q.pop();

         if (NotVisited(step.end))
         {
            // save visited tile
            steps[step.end.x][step.end.y] = step;

            // check if enemy base was reached
            if(IsWinningPos(step.end, id))
            {
#if (MB6_DEBUG)
               // make path visible on the board (debug mode only)
               MarkMinPath(steps, step.end, id);
#endif
               // found min path
               return step.pathLen;
            }

            // go through neighbour tiles and add them to the queue if the min path to them was not found yet
            for (auto& dir : directions)
            {
               if (!HasWallTowards(step.end, dir) && NotVisited(step.end + dir))
               {
                  q.push(Step_t{step.end, (step.end + dir), (uint8_t)(step.pathLen + 1)});
               }
            }
         }
      }

      // q is empty => no path exists
      return INFINITE_LEN;
   }

   bool MB6_Board::HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const
   {
      return (mBoard[pos.x][pos.y] & dirToWallMap.at(dir));
   }

#if (MB6_DEBUG)
   void MB6_Board::MarkMinPath(const Step_t steps[][qcore::BOARD_SIZE], const qcore::Position& end, const qcore::PlayerId& id)
   {
      qcore::Position pos = end;
      do
      {
         mBoard[pos.x][pos.y] |= (id == MB6_Player::_mMyId ? MARKER_MY_PATH : MARKER_OPP_PATH);
         pos = steps[pos.x][pos.y].start;
      } while (not(pos == mPlayerPos[id]));
   }
#endif

   bool MB6_Board::IsWinningPos(const qcore::Position& pos, const qcore::PlayerId& id) const
   {
      // row 0 is enemy base for me, last row is enemy base for opponent
      return ((id == MB6_Player::_mMyId && pos.x == 0) || (id != MB6_Player::_mMyId && pos.x == (qcore::BOARD_SIZE - 1))); 
   }

} // end namespace