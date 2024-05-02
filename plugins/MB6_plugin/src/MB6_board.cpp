#include "MB6_board.h"
#include "MB6_logger.h"
#include <queue>
#include <unordered_map>
#include <vector>

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

   static const std::unordered_map<ActionType_t, qcore::Orientation> actTypeToOriMap = 
   {
      {ACT_TYPE_H_WALL, qcore::Orientation::Horizontal},
      {ACT_TYPE_V_WALL, qcore::Orientation::Vertical}
   };

   // Init static members
   qcore::PlayerId MB6_Board::_mMyId = UNDEFINED_PLAYER_ID;
   qcore::PlayerId MB6_Board::_mOppId = UNDEFINED_PLAYER_ID;
   std::vector<Action_t> MB6_Board::_mBestActs(5);

   void MB6_Board::UpdatePlayerPos(const qcore::PlayerId& id, const qcore::Position& pos)
   {
      mPlayerPos[id] = pos;
   }

   void MB6_Board::UpdateWallsLeft(const qcore::PlayerId& id, const uint8_t walls)
   {
      mWallsLeft[id] = walls;
   }

   void MB6_Board::DecrementWallsLeft(const qcore::PlayerId& id)
   {
      mWallsLeft[id]--;
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

      auto IsVisited = [&](qcore::Position pos) -> bool
      {
         return (not(steps[pos.x][pos.y].end == UNDEFINED_POSITION));
      };

      if (IsWinningPos(mPlayerPos[id], id))
      {
         return 0;
      }

      q.push(Step_t{UNDEFINED_POSITION, mPlayerPos[id], 0}); // initial step from nowhere to player's pos, path len = 0

      // breadth-first-search: (if queue ever gets empty then there is no valid path from player to enemy base)
      while (!q.empty())
      {
         Step_t step = q.front();
         q.pop();

         if (not(IsVisited(step.end)))
         {
            // save visited tile
            steps[step.end.x][step.end.y] = step;

            // check if enemy base was reached
            if(IsWinningPos(step.end, id))
            {
#if (DEBUG_BOARD)
               // make path visible on the board (debug mode only)
               MarkMinPath(steps, step.end, id);
#endif
               // found min path
               return step.pathLen;
            }

            // go through neighbour tiles and add them to the queue if the min path to them was not found yet
            for (auto& dir : directions)
            {
               if (not(HasWallTowards(step.end, dir)) and not(IsVisited(step.end + dir)))
               {
                  q.push(Step_t{step.end, (step.end + dir), (uint8_t)(step.pathLen + 1)});
               }
            }
         }
      }

      // q is empty => no path exists
      return INEXISTENT_PATH;
   }

   bool MB6_Board::HasWallTowards(const qcore::Position& pos, const qcore::Direction& dir) const
   {
      return (mBoard[pos.x][pos.y] & dirToWallMap.at(dir));
   }

   bool MB6_Board::IsWinningPos(const qcore::Position& pos, const qcore::PlayerId& id) const
   {
      // row 0 is enemy base for me, last row is enemy base for opponent
      return ((id == _mMyId and pos.x == 0) or (id == _mOppId and pos.x == (qcore::BOARD_SIZE - 1))); 
   }

   bool MB6_Board::IsGameOver() const
   {
      // row 0 is enemy base for me, last row is enemy base for opponent
      return ((mPlayerPos[_mMyId].x == 0) or (mPlayerPos[_mOppId].x == (qcore::BOARD_SIZE - 1))); 
   }

   int MB6_Board::StaticEval()
   {
      uint8_t myMinPath = GetMinPath(_mMyId);
      uint8_t oppMinPath = GetMinPath(_mOppId);

      if ((myMinPath == INEXISTENT_PATH) or (oppMinPath == INEXISTENT_PATH))
      {
         return OUT_OF_RANGE;
      }
      else
      {
         // compute score: the higher the better for me
         int winTheGameScore = (myMinPath == 0 ? 1 : (oppMinPath == 0? -1 : 0)); // relevant only if a min path is 0
         int pathScore = oppMinPath - myMinPath;
         int wallScore = mWallsLeft[_mMyId] - mWallsLeft[_mOppId];
         int closerToEnemyBaseScore = ((qcore::BOARD_SIZE - 1) - mPlayerPos[_mOppId].x) - mPlayerPos[_mMyId].x;

         return (winTheGameScore * 100000 + pathScore * 1000 + wallScore * 10 + closerToEnemyBaseScore);
      }
   }

   std::vector<Action_t> MB6_Board::GetPossibleActions(const qcore::PlayerId& id) const
   {
      std::vector<Action_t> possAct;
      qcore::Position playerPos = mPlayerPos[id];
      qcore::Position enemyPos = mPlayerPos[(id ? 0 : 1)];
      
      // Add possible moves
      for (auto& dir : directions)
      {
         if (not(HasWallTowards(playerPos, dir))) // No wall between player and neighbour tile
         {
            if(not(enemyPos == playerPos + dir)) // Enemy is not on the neighbour tile
            {
               possAct.push_back({ACT_TYPE_MOVE, (playerPos + dir)}); // Moving to the neighbour tile is possible
            }
            else // Enemy is on the neighbour tile
            {
               if (not(HasWallTowards(enemyPos, dir))) // No wall between enemy and target tile
               {
                  possAct.push_back({ACT_TYPE_MOVE, (enemyPos + dir)}); // Straight jump over enemy possible
               }
               if (not(HasWallTowards(enemyPos, qcore::rotate(dir, 1)))) // No wall between enemy and diagonal tile
               {
                  possAct.push_back({ACT_TYPE_MOVE, (enemyPos + qcore::rotate(dir, 1))}); // Diag jump over enemy possible
               }
               if (not(HasWallTowards(enemyPos, qcore::rotate(dir, 3)))) // No wall between enemy and diagonal tile
               {
                  possAct.push_back({ACT_TYPE_MOVE, (enemyPos + qcore::rotate(dir, 3))}); // Diag jump over enemy possible
               }
            }
         }
      }

      // Add possible walls
      if (mWallsLeft[id] > 0)
      {
         for (int8_t i = 0; i < qcore::BOARD_SIZE - 1; i++)
         {
            for (int8_t j = 0; j < qcore::BOARD_SIZE - 1; j++)
            {
               // H Wall is possible if: No H wall here + No H wall on the tile to the right + No V wall here
               if (not(mBoard[i][j] & WALL_S) and not(mBoard[i][j + 1] & WALL_S1) and not(mBoard[i][j] & WALL_E1))
               {
                  possAct.push_back({ACT_TYPE_H_WALL, {i, j}});
               }               
               // V Wall is possible if: No V wall here + No V wall starts on the tile below + No H wall here
               if (not(mBoard[i][j] & WALL_E) and not(mBoard[i + 1][j] & WALL_E1) and not(mBoard[i][j] & WALL_S1))
               {
                  possAct.push_back({ACT_TYPE_V_WALL, {i, j}});
               }
            }
         }
      }

      return possAct;
   }

   int MB6_Board::Minimax(const qcore::PlayerId& id, const uint8_t level, int alpha, int beta, const MB6_Timer& timer, 
      const bool useTimer, bool& timerHasElapsed)
   {
      static MB6_Logger logger;
      int score;

      if (IsGameOver() or (level == 0))
      {
         score = StaticEval();
         return score;
      }

      score = (id == _mMyId ? NEG_INFINITY : POS_INFINITY);
      std::vector<Action_t> possAct = GetPossibleActions(id);

      if (possAct.empty())
      {
         return score;
      }

      for (auto& act : possAct)
      {
         MB6_Board newBoard(*this);

         if (act.actionType == ACT_TYPE_MOVE)
         {
            newBoard.UpdatePlayerPos(id, act.position);
         }
         else
         {
            newBoard.PlaceWall(act.position, actTypeToOriMap.at(act.actionType));
            newBoard.DecrementWallsLeft(id);
         }
         
         int newScore = newBoard.Minimax((id == _mMyId ? _mOppId : _mMyId), (level - 1), alpha, beta, timer, useTimer, timerHasElapsed);

         if (useTimer and timerHasElapsed)
         {
            return score;
         }

         if (IS_SCORE_VALID(newScore))
         {
            if (id == _mMyId)
            {
               if (newScore > score)
               {
                  score = newScore;
                  _mBestActs[level] = act;
               }

               if (newScore > alpha)
               {
                  alpha = newScore;
               }
            }
            else
            {
               if (newScore < score)
               {
                  score = newScore;
                  _mBestActs[level] = act;
               }

               if (newScore < beta)
               {
                  beta = newScore;
               }
            }

            if (beta <= alpha)
            {
               break;
            }
         }
      }

      return score;
   }

   Action_t MB6_Board::GetBestAction(const uint8_t level)
   {
      return _mBestActs.at(level);
   }

#if (DEBUG_BOARD)
   void MB6_Board::MarkMinPath(const Step_t steps[][qcore::BOARD_SIZE], 
                               const qcore::Position& end,
                               const qcore::PlayerId& id)
   {
      qcore::Position pos = end;
      do
      {
         mBoard[pos.x][pos.y] |= (id == _mMyId ? MARKER_MY_PATH : MARKER_OPP_PATH);
         pos = steps[pos.x][pos.y].start;
      } while (not(pos == mPlayerPos[id]));
   }
#endif

} // end namespace