#ifndef Header_qplugin_drma_minimax
#define Header_qplugin_drma_minimax

#include <stdint.h>
#include "board.h"

namespace qplugin_drma {

// these values are chosen arbitrarily, so that (ERROR_NO_PATH < NEG_INFINITY < BEST_NEG_SCORE) and (BEST_POS_SCORE < POS_INFINITY)
#define ERROR_NO_PATH   (-0xFFFFFFF)
#define POS_INFINITY    (+0xFFFFFF)
#define NEG_INFINITY    (-0xFFFFFF)
#define BEST_POS_SCORE  (+0xFFFFF0)
#define BEST_NEG_SCORE  (-0xFFFFF0)

int Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta);
Play_t GetBestPlayForLevel(uint8_t level);

}
#endif // Header_qplugin_drma_minimax
