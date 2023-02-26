#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "min_path.h"

#define INFINITE_LEN     0xFFU
#define QUEUE_MAX_SIZE   1000


static void QueueInit(void);
static bool IsQueueEmpty(void);
static void QueuePush(Subpath_t item);
static Subpath_t* QueuePop(void);
static void FoundSubpathsInit(void);
static bool IsMinPathFoundForTile(Tile_t* tile);

static Subpath_t queue[QUEUE_MAX_SIZE];
static uint16_t queueNext, queueFirst;
static Subpath_t foundSubpaths[BOARD_SZ][BOARD_SZ];
static uint16_t goalTilesReached;
static Subpath_t* destination;


uint8_t FindMinPathLen(Board_t* board, Player_t player, bool* found)
{
    if (GetPlayerTile(board, player)->isGoalFor == player)
    {
        // player is in the enemy base
        *found = true;
        return 0;
    }

    QueueInit();
    FoundSubpathsInit();

    uint8_t minPathLen = INFINITE_LEN;

    // start with the player's tile
    Subpath_t source = 
    { 
        GetPlayerTile(board, player), 
        NULL, 
        0 
    };

    QueuePush(source);

    // Breadth-First-Search. 
    // Stops when min path was found for a goal tile or when queue empty (meaning there is no path).
    while((goalTilesReached == 0) && !IsQueueEmpty())
    {
        Subpath_t* item = QueuePop();

        int8_t x = item->tile->pos.x;
        int8_t y = item->tile->pos.y;

        // if min path was not yet found for the current tile
        if (foundSubpaths[x][y].tile == NULL)
        {
            // save path info
            memcpy(&(foundSubpaths[x][y]), item, sizeof(Subpath_t));

            // if the tile reached is a goal-tile for the current player
            if (item->tile->isGoalFor == player)
            {
                // flag it as reached
                goalTilesReached |= (1U << y);

                // update min path length
                if (minPathLen > item->pathLen)
                {
                    minPathLen = item->pathLen;
                    destination = item; // debug
                }
            }

            // go through neighbour tiles and add them to the queue if min path to them was not found yet
            if (item->tile->north && !IsMinPathFoundForTile(item->tile->north))
            {
                Subpath_t newItem = 
                {
                    item->tile->north, 
                    item->tile, 
                    ((uint8_t)(item->pathLen + 1))
                };
                QueuePush(newItem);
            }

            if (item->tile->west && !IsMinPathFoundForTile(item->tile->west))
            {
                Subpath_t newItem = 
                {
                    item->tile->west, 
                    item->tile, 
                    ((uint8_t)(item->pathLen + 1))
                };
                QueuePush(newItem);
            }

            if (item->tile->east && !IsMinPathFoundForTile(item->tile->east))
            {
                Subpath_t newItem = 
                {
                    item->tile->east, 
                    item->tile, 
                    ((uint8_t)(item->pathLen + 1))
                };
                QueuePush(newItem);
            }

            if (item->tile->south && !IsMinPathFoundForTile(item->tile->south))
            {
                Subpath_t newItem = 
                {
                    item->tile->south, 
                    item->tile, 
                    ((uint8_t)(item->pathLen + 1))
                };
                QueuePush(newItem);
            }
        }
    }
    
    #if (SHOW_MIN_PATH_ON_LOGGED_BOARD)
        // ---------------------------------------------------------------------------------
        // debug - delete prev tiles marked as being on the min path
        memset(board->debug_isOnMinPath, 0, sizeof(board->debug_isOnMinPath));
        // debug - backtrack from destination tile to mark all tiles that are part of min path
        Tile_t* stopAt = &(board->tiles[board->playerPos[player].x][board->playerPos[player].y]);
        Tile_t* current = destination->tile;
        while (current != stopAt)
        {
            board->debug_isOnMinPath[current->pos.x][current->pos.y] = true;
            current = foundSubpaths[current->pos.x][current->pos.y].prevTile;
        }
        // end debug ---------------------------------------------------------------------------
    #endif

    *found = (!!goalTilesReached);

    return minPathLen;
}

static void QueueInit(void)
{
    queueNext = 0;
    queueFirst = 0;
}

static bool IsQueueEmpty(void)
{
    return (queueNext == queueFirst);
}

static void QueuePush(Subpath_t item)
{
    queue[queueNext++] = item;
}

static Subpath_t* QueuePop(void)
{
    return &(queue[queueFirst++]);
}

static void FoundSubpathsInit(void)
{
    memset(foundSubpaths, 0, sizeof(foundSubpaths));
    goalTilesReached = 0;
}

static bool IsMinPathFoundForTile(Tile_t* tile)
{
    return (foundSubpaths[tile->pos.x][tile->pos.y].pathLen != 0);
}
