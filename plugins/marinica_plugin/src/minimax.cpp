#include <stdbool.h>
#include "board.h"
#include "min_path.h"
#include "minimax.h"

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



int minimax(Board_t* board, Player_t player, uint8_t level, BestPlay_t* bestPlay)
{
    uint8_t myMinPath;
    uint8_t oppMinPath;
    bool foundMinPathMe;
    bool foundMinPathOpp;
    int score;

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
        myMinPath = FindMinPathLen(board, ME, &foundMinPathMe);
        oppMinPath = FindMinPathLen(board, OPPONENT, &foundMinPathOpp);

        return ((!foundMinPathMe) || (!foundMinPathOpp) ? ERROR_NO_PATH : (oppMinPath - myMinPath));
    }
    else
    {
        score = (player == ME ? NEG_INFINITY : POS_INFINITY); // initialize to worst possible score

        Wall_t* wall;
        FOREACH_PERMITTED_WALL_AS(wall)
        {
            PlaceWall(board, player, wall);

            BestPlay_t play;
            int tempScore = minimax(board, board->otherPlayer[player], level - 1, &play);
            if (IS_VALID(tempScore) && ((player == ME && tempScore > score) || (player == OPPONENT && tempScore < score)))
            {
                score = tempScore;
                *bestPlay = play;
            }

            UndoWall(board, player, wall);

            UpdatePossibleMoves(board, ME);
            UpdatePossibleMoves(board, OPPONENT);
        }
        END_FOREACH_PERMITTED_WALL;

        MoveID_t move;
        FOREACH_POSSIBLE_MOVE_AS(move)
        {
            MakeMove(board, player, move);

            BestPlay_t play;
            int tempScore = minimax(board, board->otherPlayer[player], level - 1, &play);
            if (IS_VALID(tempScore) && ((player == ME && tempScore > score) || (player == OPPONENT && tempScore < score)))
            {
                score = tempScore;
                *bestPlay = play;
            }

            UndoMove(board, player, move);

            UpdatePossibleMoves(board, ME);
            UpdatePossibleMoves(board, OPPONENT);
        }
        END_FOREACH_POSSIBLE_MOVE;
    }

    return score;
}
