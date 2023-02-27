#ifndef Header_qplugin_marinica_minimax
#define Header_qplugin_marinica_minimax

#include <stdint.h>
#include "board.h"

namespace qplugin {

int Minimax(Board_t* board, Player_t player, uint8_t level);
Play_t GetBestPlayForLevel(uint8_t level);
}

#endif // Header_qplugin_marinica_minimax
