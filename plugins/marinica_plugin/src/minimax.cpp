#include <stdbool.h>
#include <stddef.h>
#include "board.h"
#include "min_path.h"
#include "minimax.h"

#define MAX_RECURSIVE_LEVELS 6 // not achievable

// these values are chosen arbitrarily, so that (ERROR_NO_PATH < NEG_INFINITY < BEST_NEG_SCORE) and (BEST_POS_SCORE < POS_INFINITY)
#define ERROR_NO_PATH   (-0xFFFFFFF)
#define POS_INFINITY    (+0xFFFFFF)
#define NEG_INFINITY    (-0xFFFFFF)
#define BEST_POS_SCORE  (+0xFFFFF0)
#define BEST_NEG_SCORE  (-0xFFFFF0)

#define IS_VALID(score) (BEST_NEG_SCORE <= score && score <= BEST_POS_SCORE)

// yeah, I know.. but it keeps minimax function readable
#define FOREACH_PERMITTED_WALL_AS(wall)     for (int o = H; o <= V; o++) { \
                                                for (int i = 0; i < BOARD_SZ - 1; i++) { \
                                                    for (int j = 0; j < BOARD_SZ - 1; j++) { \
                                                        wall = &(board->walls[o][i][j]); \
                                                            if (wall->permission == WALL_PERMITTED)
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

        return (pathScore * 100 + wallScore);       
    }
}


int Minimax(Board_t* board, Player_t player, uint8_t level)
{
    UpdatePossibleMoves(board, ME);
    UpdatePossibleMoves(board, OPPONENT);

    if (HasPlayerWon(board, ME))
    {
        return BEST_POS_SCORE;
    }
    else if (HasPlayerWon(board, OPPONENT))
    {
        return BEST_NEG_SCORE;
    }
    else if (level == 0)
    {
        return StaticEval(board);
    }
    else
    {
        int score = (player == ME ? NEG_INFINITY : POS_INFINITY); // initialize to worst possible score

        if (board->wallsLeft[ME])
        {
            Wall_t* wall;
            FOREACH_PERMITTED_WALL_AS(wall)
            {
                PlaceWall(board, player, wall);

                int tempScore = Minimax(board, board->otherPlayer[player], level - 1);
                if (IS_VALID(tempScore) && ((player == ME && tempScore > score) || (player == OPPONENT && tempScore < score)))
                {
                    score = tempScore;
                    bestPlays[level] = { PLACE_WALL, NULL_MOVE, wall };
                }

                UndoWall(board, player, wall);

                UpdatePossibleMoves(board, ME);
                UpdatePossibleMoves(board, OPPONENT);
            }
            END_FOREACH_PERMITTED_WALL;
        }

        MoveID_t move;
        FOREACH_POSSIBLE_MOVE_AS(move)
        {
            MakeMove(board, player, move);

            int tempScore = Minimax(board, board->otherPlayer[player], level - 1);
            if (IS_VALID(tempScore) && ((player == ME && tempScore > score) || (player == OPPONENT && tempScore < score)))
            {
                score = tempScore;
                bestPlays[level] = { MAKE_MOVE, move, NULL };
            }

            UndoMove(board, player, move);

            UpdatePossibleMoves(board, ME);
            UpdatePossibleMoves(board, OPPONENT);
        }
        END_FOREACH_POSSIBLE_MOVE;

        return score;
    }
}

Play_t GetBestPlayForLevel(uint8_t level)
{
    return bestPlays[level];
}