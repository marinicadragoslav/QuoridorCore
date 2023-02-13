#ifndef Header_qplugin_marinica_min_path
#define Header_qplugin_marinica_min_path

#include <stdint.h>
#include "board.h"

#define SHOW_MIN_PATH_ON_LOGGED_BOARD 0

typedef struct
{
   Tile_t* tile;
   Tile_t* prevTile;
   uint8_t pathLen;
}Subpath_t;

uint8_t FindMinPathLen(Player_t player);
uint16_t debug_GetReachedGoalTiles(void);

#endif // Header_qplugin_marinica_min_path
