#ifndef Header_qplugin_marinica_min_path
#define Header_qplugin_marinica_min_path

#include <stdint.h>
#include "board.h"

typedef struct
{
   Tile_t* tile;
   Tile_t* prevTile;
   uint8_t pathLen;
}Path_t;



#endif // Header_qplugin_marinica_min_path
