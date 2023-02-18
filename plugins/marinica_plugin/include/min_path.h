#ifndef Header_qplugin_marinica_min_path
#define Header_qplugin_marinica_min_path

#include <stdint.h>
#include <stdbool.h>
#include "board.h"

#define SHOW_MIN_PATH_ON_LOGGED_BOARD  false

typedef struct
{
   Tile_t* tile;
   Tile_t* prevTile;
   uint8_t pathLen;
}Subpath_t;

uint8_t FindMinPathLen(Board_t* board, Player_t player, bool* found);

#endif // Header_qplugin_marinica_min_path
