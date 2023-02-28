#include <stdbool.h>
#include <stddef.h>
#include "board.h"
#include "min_path.h"
#include "minimax.h"
#include <chrono>

namespace qplugin_drma {

extern bool isSecondMinimaxPassInterrupted;
extern bool isFirstMinimaxPass;
extern std::chrono::time_point<std::chrono::steady_clock> t0;

#define MAX_RECURSIVE_LEVELS 6 // not achievable

#define IS_VALID(score) (BEST_NEG_SCORE <= score && score <= BEST_POS_SCORE)

// yeah, I know.. but it keeps minimax function readable
#define FOREACH_PERMITTED_WALL_AS(wall)     for (int o = V; o >= H; o--) { \
                                                for (int i = 0; i < BOARD_SZ - 1; i++) { \
                                                    for (int j = 0; j < BOARD_SZ - 1; j++) { \
                                                        wall = &(board->walls[o][i][j]); \
                                                            if (wall->permission == WALL_PERMITTED && (!wall->isDisabled))
#define END_FOREACH_PERMITTED_WALL           } } }

#define FOREACH_POSSIBLE_MOVE_AS(move)      for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++) { \
                                                if (board->moves[player][moveID].isPossible) { \
                                                    move = (MoveID_t)moveID;
#define END_FOREACH_POSSIBLE_MOVE           } }

// stores the best play for every level. Get it with GetBestPlayForLevel(level);
static Play_t bestPlays[MAX_RECURSIVE_LEVELS]; 

// Static evaluation of the board position, from my perspective (maximizing player) => a high score is good for me
static int StaticEval(Board_t* board)
{
    bool foundMinPathMe;
    bool foundMinPathOpp;

    uint8_t myMinPath = FindMinPathLen(board, ME, &foundMinPathMe);
    uint8_t oppMinPath = FindMinPathLen(board, OPPONENT, &foundMinPathOpp);

    if ((!foundMinPathMe) || (!foundMinPathOpp))
    {
         // there is no valid path for one of the players in the current position
        return ERROR_NO_PATH;
    }
    else
    {
        int pathScore = oppMinPath - myMinPath;
        int wallScore = board->wallsLeft[ME] - board->wallsLeft[OPPONENT];
        int closerToEnemyBaseScore = ((BOARD_SZ - 1) - board->playerPos[OPPONENT].x) - board->playerPos[ME].x;

        return (pathScore * 1000 + wallScore * 10 + closerToEnemyBaseScore);
    }
}


int Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta)
{
    UpdatePossibleMoves(board, ME);
    UpdatePossibleMoves(board, OPPONENT);

    if (HasPlayerWon(board, ME) || HasPlayerWon(board, OPPONENT) || (level == 0))
    {
        return StaticEval(board);
    }
    else
    {
        int score = (player == ME ? NEG_INFINITY : POS_INFINITY); // initialize to worst possible score

        if (board->wallsLeft[player])
        {
            Wall_t* wall;
            FOREACH_PERMITTED_WALL_AS(wall)
            {
                bool prune = false;
                PlaceWall(board, player, wall);

                int tempScore = Minimax(board, board->otherPlayer[player], level - 1, alpha, beta);
                if (IS_VALID(tempScore))
                {
                    if(player == ME)
                    {
                        if (tempScore > score)
                        {
                            score = tempScore;
                            bestPlays[level] = { PLACE_WALL, NULL_MOVE, wall };
                        }

                        if (tempScore > alpha)
                        {
                            alpha = tempScore;
                        }
                    }
                    else
                    {
                        if (tempScore < score)
                        {
                            score = tempScore;
                            bestPlays[level] = { PLACE_WALL, NULL_MOVE, wall };
                        }

                        if (tempScore < beta)
                        {
                            beta = tempScore;
                        }
                    }

                    if (beta <= alpha)
                    {
                        prune = true;
                    }
                }         

                UndoWall(board, player, wall);

                UpdatePossibleMoves(board, ME);
                UpdatePossibleMoves(board, OPPONENT);

                if (prune)
                {
                    goto Exit_walls_loop;
                }

                if (!isFirstMinimaxPass && (level == 1))
                {
                    std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
                    if (((std::chrono::duration<double>)(t1 - t0)).count() > 4.5)
                    {
                        isSecondMinimaxPassInterrupted = true;
                        goto Exit_walls_loop;
                    }
                }
            }
            END_FOREACH_PERMITTED_WALL;
        }

        Exit_walls_loop:

        MoveID_t move;
        FOREACH_POSSIBLE_MOVE_AS(move)
        {
            bool prune = false;
            MakeMove(board, player, move);

            int tempScore = Minimax(board, board->otherPlayer[player], level - 1, alpha, beta);
            if (IS_VALID(tempScore))
            {
                if(player == ME)
                {
                    if (tempScore > score)
                    {
                        score = tempScore;
                        bestPlays[level] = { MAKE_MOVE, move, NULL };
                    }

                    if (tempScore > alpha)
                    {
                        alpha = tempScore;
                    }
                }
                else
                {
                    if (tempScore < score)
                    {
                        score = tempScore;
                        bestPlays[level] = { MAKE_MOVE, move, NULL };
                    }

                    if (tempScore < beta)
                    {
                        beta = tempScore;
                    }
                }

                if (beta <= alpha)
                {
                    prune = true;
                }
            }         

            UndoMove(board, player, move);

            UpdatePossibleMoves(board, ME);
            UpdatePossibleMoves(board, OPPONENT);

            if (prune)
            {
                goto Exit_moves_loop;
            }

            if (!isFirstMinimaxPass && (level == 1))
            {
                std::chrono::time_point<std::chrono::steady_clock> t1 = std::chrono::steady_clock::now();
                if (((std::chrono::duration<double>)(t1 - t0)).count() > 4.5)
                {
                    isSecondMinimaxPassInterrupted = true;
                    goto Exit_moves_loop;
                }
            }
        }
        END_FOREACH_POSSIBLE_MOVE;

        Exit_moves_loop:

        return score;
    }
}

Play_t GetBestPlayForLevel(uint8_t level)
{
    return bestPlays[level];
}

}