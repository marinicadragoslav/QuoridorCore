
#include <stdio.h>
#include <string.h>
#include "QcoreUtil.h"
#include "debug.h"

#define LOGLEN 120

static void ClearBuff(void);
static const char * const DOM = "qplugin::MP";


static char buff[LOGLEN];

void debug_PrintTileStructure(Board_t* board)
{
    for (uint8_t x = 0; x < BOARD_SZ; x++)
    {
        for (uint8_t y = 0; y < BOARD_SZ; y++)
        {
            Tile_t tile = board->tiles[x][y];

            ClearBuff();
            sprintf(buff, "Tile [%u, %u]: ", tile.pos.x, tile.pos.y);
            if (tile.north) sprintf(buff + strlen(buff), ".N = [%u, %u], ", (tile.north)->pos.x, (tile.north)->pos.y); else sprintf(buff + strlen(buff), ".N = [NULL], ");
            if (tile.west) sprintf(buff + strlen(buff), ".W = [%u, %u], ", (tile.west)->pos.x, (tile.west)->pos.y); else sprintf(buff + strlen(buff), ".W = [NULL], ");
            if (tile.east) sprintf(buff + strlen(buff), ".E = [%u, %u], ", (tile.east)->pos.x, (tile.east)->pos.y); else sprintf(buff + strlen(buff), ".E = [NULL], ");
            if (tile.south) sprintf(buff + strlen(buff), ".S = [%u, %u], ", (tile.south)->pos.x, (tile.south)->pos.y); else sprintf(buff + strlen(buff), ".S = [NULL], ");
            
            if (tile.isGoalForMe) sprintf(buff + strlen(buff), "GfM");
            if (tile.isGoalForOpp) sprintf(buff + strlen(buff), "GfO");

            LOG_WARN(DOM) << buff;
        }
    }
}

void debug_PrintWallHStructure(Board_t* board)
{
    for (uint8_t x = 0; x < BOARD_SZ - 1; x++)
    {
        for (uint8_t y = 0; y < BOARD_SZ - 1; y++)
        {
            HWall_t w = board->hWalls[x][y];

            ClearBuff();
            sprintf(buff, "Hw [%u, %u]: ", w.pos.x, w.pos.y);
            if (w.west) sprintf(buff + strlen(buff), ".wW [%u, %u], ", (w.west)->pos.x, (w.west)->pos.y); else sprintf(buff + strlen(buff), ".wW [NULL], ");
            if (w.east) sprintf(buff + strlen(buff), ".wE [%u, %u], ", (w.east)->pos.x, (w.east)->pos.y); else sprintf(buff + strlen(buff), ".wE [NULL], ");

            Tile_t* nw = w.northwest;
            Tile_t* ne = w.northeast;
            Tile_t* sw = w.southwest;
            Tile_t* se = w.southeast;

            sprintf(buff + strlen(buff), ".tNW [%u, %u], ", nw->pos.x, nw->pos.y);
            sprintf(buff + strlen(buff), ".tNE [%u, %u], ", ne->pos.x, ne->pos.y);
            sprintf(buff + strlen(buff), ".tSW [%u, %u], ", sw->pos.x, sw->pos.y);
            sprintf(buff + strlen(buff), ".tSE [%u, %u], ", se->pos.x, se->pos.y);

            sprintf(buff + strlen(buff), ".pF %u, ", w.possibleFlag);
            if (w.debug_isPossible) sprintf(buff + strlen(buff), "P"); else sprintf(buff + strlen(buff), "X");

            LOG_WARN(DOM) << buff;
        }
    }
}

void debug_PrintWallVStructure(Board_t* board)
{
    for (uint8_t x = 0; x < BOARD_SZ - 1; x++)
    {
        for (uint8_t y = 0; y < BOARD_SZ - 1; y++)
        {
            VWall_t w = board->vWalls[x][y];

            ClearBuff();
            sprintf(buff, "Vw [%u, %u]: ", w.pos.x, w.pos.y);
            if (w.north) sprintf(buff + strlen(buff), ".wN [%u, %u], ", (w.north)->pos.x, (w.north)->pos.y); else sprintf(buff + strlen(buff), ".wN [NULL], ");
            if (w.south) sprintf(buff + strlen(buff), ".wS [%u, %u], ", (w.south)->pos.x, (w.south)->pos.y); else sprintf(buff + strlen(buff), ".wS [NULL], ");

            Tile_t* nw = w.northwest;
            Tile_t* ne = w.northeast;
            Tile_t* sw = w.southwest;
            Tile_t* se = w.southeast;

            sprintf(buff + strlen(buff), ".tNW [%u, %u], ", nw->pos.x, nw->pos.y);
            sprintf(buff + strlen(buff), ".tNE [%u, %u], ", ne->pos.x, ne->pos.y);
            sprintf(buff + strlen(buff), ".tSW [%u, %u], ", sw->pos.x, sw->pos.y);
            sprintf(buff + strlen(buff), ".tSE [%u, %u], ", se->pos.x, se->pos.y);

            sprintf(buff + strlen(buff), ".pF %u, ", w.possibleFlag);
            if (w.debug_isPossible) sprintf(buff + strlen(buff), "P"); else sprintf(buff + strlen(buff), "X");

            LOG_WARN(DOM) << buff;
        }
    }
}

static void ClearBuff(void)
{
    memset(buff, 0, sizeof(buff));
}

