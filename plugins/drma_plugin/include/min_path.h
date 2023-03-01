#ifndef Header_qplugin_drma_min_path
#define Header_qplugin_drma_min_path

#include <stdint.h>
#include <stdbool.h>
#include "board.h"
#include "drma_player.h"

namespace qplugin_drma 
{

   typedef struct
   {
      Tile_t* tile;
      Tile_t* prevTile;
      uint8_t pathLen;
   }Subpath_t;

   uint8_t FindMinPathLen(Board_t* board, Player_t player, bool* found);

} // end namespace
#endif // Header_qplugin_drma_min_path
