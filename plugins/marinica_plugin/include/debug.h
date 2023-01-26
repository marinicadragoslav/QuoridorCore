#ifndef Header_qplugin_marinica_debug
#define Header_qplugin_marinica_debug

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

void debug_PrintTileStructure(Board_t* board);
void debug_PrintWallHStructure(Board_t* board);
void debug_PrintWallVStructure(Board_t* board);
void debug_PrintPossibleHWallsList(Board_t* board);
void debug_PrintPossibleVWallsList(Board_t* board);
void debug_PrintTestFailed(void);
void debug_PrintTestPassed(void);
void debug_PrintTestErrorMsg(const char* errMsg);
void debug_PrintTestMessage(const char* msg);

#endif // Header_qplugin_marinica_debug
