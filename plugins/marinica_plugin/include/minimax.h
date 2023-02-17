#ifndef Header_qplugin_marinica_minimax
#define Header_qplugin_marinica_minimax

#include <stdint.h>
#include "board.h"

int minimax(Board_t* board, Player_t player, uint8_t level, BestPlay_t* bestPlay);

#endif // Header_qplugin_marinica_minimax
