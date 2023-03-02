#include <stdbool.h>
#include <stddef.h>
#include "board.h"
#include "min_path.h"
#include "minimax.h"
#include "drma_player.h"
#include <chrono>

#define IS_VALID(score) (CAP_NEG_SCORE <= score && score <= CAP_POS_SCORE)

namespace qplugin_drma 
{
    // stores the best play for every level. Get it with GetBestPlayForLevel(level);
    static Play_t bestPlays[MINIMAX_DEPTH + 1]; 

    // Static evaluation of the board position, from my perspective (maximizing player). A high score is good for me.
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
            int winTheGameScore = (myMinPath == 0 ? 1 : (oppMinPath == 0? -1 : 0));
            int pathScore = oppMinPath - myMinPath;
            int wallScore = board->wallsLeft[ME] - board->wallsLeft[OPPONENT];
            int closerToEnemyBaseScore = ((BOARD_SZ - 1) - board->playerPos[OPPONENT].x) - board->playerPos[ME].x;

            return (winTheGameScore * 100000 + pathScore * 1000 + wallScore * 10 + closerToEnemyBaseScore);
        }
    }

    int Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta,
                std::chrono::time_point<std::chrono::steady_clock> tStart, bool canTimeOut, bool *hasTimedOut)
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

            if (canTimeOut && (level == 1))
            {
                std::chrono::time_point<std::chrono::steady_clock> tNow = std::chrono::steady_clock::now();
                if (std::chrono::duration_cast<std::chrono::milliseconds>(tNow - tStart).count() > MINIMAX_TIMEOUT_MS)
                {
                    *hasTimedOut = true;
                    return score;
                }
            }

            for (uint32_t i = 0; i < (sizeof(board->plays) / sizeof(board->plays[0])); i++)
            {
                NominalPlay_t play = board->plays[i];
                bool prune = false;
                Action_t currentAction = NULL_ACTION;
                MoveID_t currentMoveID = NULL_MOVE;
                Wall_t* currentWall = NULL;

                if ((play.action == PLACE_WALL) && (board->wallsLeft[player]) && 
                        (play.wall->permission == WALL_PERMITTED) && (play.wall->isEnabled))
                {
                    PlaceWall(board, player, play.wall);
                    currentAction = PLACE_WALL;
                    currentWall = play.wall;
                }
                else if (play.action == MAKE_MOVE && play.player == player && play.move->isPossible)
                {
                    MakeMove(board, player, play.move->moveID);
                    currentAction = MAKE_MOVE;
                    currentMoveID = play.move->moveID;
                }

                if (currentAction != NULL_ACTION)
                {
                    int tempScore = Minimax(board, board->otherPlayer[player], level - 1, alpha, beta, tStart, canTimeOut, hasTimedOut);

                    if (IS_VALID(tempScore) && (!canTimeOut || (canTimeOut && !(*hasTimedOut))))
                    {
                        if(player == ME)
                        {
                            if (tempScore > score)
                            {
                                score = tempScore;
                                bestPlays[level] = { currentAction, currentMoveID, currentWall};
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
                                bestPlays[level] = { currentAction, currentMoveID, currentWall };
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

                    if (currentAction == PLACE_WALL)
                    {
                        UndoWall(board, player, play.wall);
                    }
                    else
                    {
                        UndoMove(board, player, play.move->moveID);
                    }

                    UpdatePossibleMoves(board, ME);
                    UpdatePossibleMoves(board, OPPONENT);

                    if (canTimeOut && (*hasTimedOut))
                    {
                        return score;
                    }

                    if (prune)
                    {
                        break;
                    }
                }
            }

            return score;
        }
    }


    Play_t GetBestPlayForLevel(uint8_t level)
    {
        return bestPlays[level];
    }

} // end namespace