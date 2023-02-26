#ifndef Header_qplugin_dragoslav_minimax
#define Header_qplugin_dragoslav_minimax

#include <stdint.h>
#include "board.h"


int Minimax(Board_t* board, Player_t player, uint8_t level);
Play_t GetBestPlayForLevel(uint8_t level);

#endif // Header_qplugin_dragoslav_minimax
