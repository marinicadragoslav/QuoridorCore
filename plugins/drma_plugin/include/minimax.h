#ifndef Header_qplugin_drma_minimax
#define Header_qplugin_drma_minimax

#include <stdint.h>
#include <chrono>
#include "board.h"

namespace qplugin_drma 
{

    int Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta, 
                    std::chrono::time_point<std::chrono::steady_clock> tStart, bool canTimeOut, bool *hasTimedOut);

    Play_t GetBestPlayForLevel(uint8_t level);

} // end namespace
#endif // Header_qplugin_drma_minimax
