#ifndef Header_qplugin_drma_debug
#define Header_qplugin_drma_debug

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

namespace qplugin_drma 
{

    #if (RUN_TESTS)
    void debug_PrintTestFailed(void);
    void debug_PrintTestPassed(void);
    void debug_PrintTestErrorMsg(const char* errMsg);
    void debug_PrintTestMessage(const char* msg);
    #endif

    char* debug_PrintMyPossibleMoves(Board_t* board);
    char* debug_PrintOppPossibleMoves(Board_t* board);
    void debug_PrintBoard(Board_t* board);
    void debug_PrintTestMinPaths(int minPathMe, int minPathOpp);
    void debug_PrintPlay(Play_t play);

} // end namespace
#endif // Header_qplugin_drma_debug
