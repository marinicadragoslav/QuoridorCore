#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "min_path.h"

#define INFINITE_LEN    0xFFU // some large number that is impossible for a path len
#define ALL_PATHS_FOUND 0x01FF // binary 111111111 (each bit means that min path was found for that particular goal tile)
#define QUEUE_MAX_SIZE  1000  // some large value that the number of queued elements will never reach

/* Queue related */
static Path_t queue[QUEUE_MAX_SIZE];
static uint16_t queueNext, queueFirst;

static void QueueInit(void);
static bool IsQueueEmpty(void);
static void QueuePush(Path_t item);
static Path_t* QueuePop(void);

/* Path related */
static void InitPathData(void);
static Path_t savedPathInfo[BOARD_SZ][BOARD_SZ];
static uint16_t pathFoundFlags;
static Path_t* debug_destination;

uint16_t debug_GetFlags(void)
{
    return pathFoundFlags;
}


uint8_t FindMinPathLen(Player_t player)
{
    QueueInit();
    InitPathData();

    Board_t* board = GetBoard();
    uint8_t minPathLen = INFINITE_LEN;

    // add source tile path info to queue
    Position_t sourcePos = (player == ME ? board->myPos : board->oppPos);
    Path_t source = { &(board->tiles[sourcePos.x][sourcePos.y]), NULL, 0 };
    QueuePush(source);

    // Breadth-First-Search that stops when either min paths were found for each of the goal tiles or
    // when queue is empty (meaning some goal tiles are unreachable)
    while((pathFoundFlags != ALL_PATHS_FOUND) && !IsQueueEmpty())
    {
        Path_t* item = QueuePop();
        Tile_t* tile = item->tile;        
        Path_t* saved = &(savedPathInfo[tile->pos.x][tile->pos.y]);

        if (saved->tile == NULL)
        {
            // min path not yet found for this tile => update min path info
            saved->tile = tile;
            saved->prevTile = item->prevTile;
            saved->pathLen = item->pathLen;

            // if the tile reached is a goal-tile for the current player, flag it as done
            if (((player == ME) && (tile->isGoalForMe)) || 
                ((player == OPPONENT) && (tile->isGoalForOpp)))
            {
                // set path-found-flag for this tile
                pathFoundFlags |= (1U << tile->pos.y);

                // update min path length
                if (minPathLen > item->pathLen)
                {
                    minPathLen = item->pathLen;
                    debug_destination = saved; // debug
                }
            }

            // go through neighbour tiles and add them to the queue if the minimum path to them was not found yet
            if (tile->north && (savedPathInfo[tile->north->pos.x][tile->north->pos.y].pathLen == 0))
            {
                QueuePush({ tile->north, tile, (uint8_t)(item->pathLen + 1U) });
            }

            if (tile->west && (savedPathInfo[tile->west->pos.x][tile->west->pos.y].pathLen == 0))
            {
                QueuePush({ tile->west, tile, (uint8_t)(item->pathLen + 1U) });
            }

            if (tile->east && (savedPathInfo[tile->east->pos.x][tile->east->pos.y].pathLen == 0))
            {
                QueuePush({ tile->east, tile, (uint8_t)(item->pathLen + 1U) });
            }

            if (tile->south && (savedPathInfo[tile->south->pos.x][tile->south->pos.y].pathLen == 0))
            {
                QueuePush({ tile->south, tile, (uint8_t)(item->pathLen + 1U) });
            }
        }
    }
    
    // ---------------------------------------------------------------------------------
    // debug - delete prev tiles marked as being on the min path
    memset(board->debug_isOnMyMinPath, 0, sizeof(board->debug_isOnMyMinPath));
    // debug - backtrack from destination tile to mark all tiles that are part of min path
    Tile_t* stopAt = (player == ME ? &(board->tiles[board->myPos.x][board->myPos.y]) : &(board->tiles[board->oppPos.x][board->oppPos.y]));
    Tile_t* current = debug_destination->tile;
    while (current != stopAt)
    {
        board->debug_isOnMyMinPath[current->pos.x][current->pos.y] = true;
        current = savedPathInfo[current->pos.x][current->pos.y].prevTile;
    }
    // end debug ---------------------------------------------------------------------------

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

static void QueuePush(Path_t item)
{
    queue[queueNext++] = item;      
}

static Path_t* QueuePop(void)
{
    return &queue[queueFirst++];
}

static void InitPathData(void)
{
    memset(savedPathInfo, 0, sizeof(savedPathInfo));
    pathFoundFlags = 0;
}
