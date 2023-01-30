
#include <stdio.h>
#include <string.h>
#include "QcoreUtil.h"
#include "debug.h"

#define LOGLEN 120

static void ClearBuff(void);
static const char* debug_ConvertMoveIDToString(MoveID_t moveID);

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

            LOG_WARN(DOM) << buff;
        }
    }
}

void debug_PrintTestMessage(const char* msg)
{
    LOG_WARN(DOM) << msg;
}

void debug_PrintTestPassed(void)
{
    LOG_WARN(DOM) << "TEST PASSED!";
}

void debug_PrintTestFailed(void)
{
    LOG_ERROR(DOM) << "TEST FAILED!";
}

void debug_PrintTestErrorMsg(const char* errMsg)
{
    LOG_ERROR(DOM) << errMsg;
}

void debug_PrintMyPossibleMoves(Board_t* board)
{    
    ClearBuff();
    sprintf(buff + strlen(buff), "  My psb mvs: ");
    
    for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
    {
        if (board->myMoves[i].isPossible)
        {
            sprintf(buff + strlen(buff), "[%s],", debug_ConvertMoveIDToString((MoveID_t)i));
        }
    }
    LOG_INFO(DOM) << buff;
}

static void ClearBuff(void)
{
    memset(buff, 0, sizeof(buff));
}

static const char* debug_ConvertMoveIDToString(MoveID_t moveID)
{   
    switch (moveID)
    {
    case MOVE_NORTH:
        return "M-N";
    case MOVE_SOUTH:
        return "M-S";
    case MOVE_WEST:
        return "M-W";
    case MOVE_EAST:
        return "M-E";
    case JUMP_NORTH:
        return "J-N";
    case JUMP_SOUTH:
        return "J-S";
    case JUMP_EAST:
        return "J-E";
    case JUMP_WEST:
        return "J-W";
    case JUMP_NORTH_EAST:
        return "J-N-E";
    case JUMP_NORTH_WEST:
        return "J-N-W";
    case JUMP_SOUTH_EAST:
        return "J-S-E";
    case JUMP_SOUTH_WEST:
        return "J-S-W";
    default:
        return "F*ck";
    }
}

void debug_PrintTile(const char* name, int8_t x, int8_t y)
{
    ClearBuff();
    sprintf(buff + strlen(buff), "%s, ", name);
    sprintf(buff + strlen(buff), "[%d, ", x);
    sprintf(buff + strlen(buff), "%d] ", y);
    LOG_INFO(DOM) << buff;
}

void debug_PrintBoard(Board_t* board)
{
    char tiles[BOARD_SZ * BOARD_SZ] = { 0 };
    char vertw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };
    char horizw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };

    uint8_t ti = 0; 
    uint8_t hi = 0;
    uint8_t vi = 0;

    for (uint8_t i = 0; i < BOARD_SZ; i++)
    {
        for (uint8_t j = 0; j < BOARD_SZ; j++)
        {
            // populate vertical wall array
            if (j < BOARD_SZ - 1) 
            {
                vertw[vi++] = ((board->tiles[i][j].east == NULL) ? '|' : ' ');
            }

            // populate horiz wall array
            if (i < BOARD_SZ - 1) 
            {
                horizw[hi++] = ((board->tiles[i][j].south == NULL) ? '-' : ' ');
            }

            // populate tiles array
            if (i == board->myPos.x && j == board->myPos.y)   { tiles[ti++] = 'M'; continue; } // 'M' for my postion
            if (i == board->oppPos.x && j == board->oppPos.y) { tiles[ti++] = 'O'; continue; } // 'O' for opponent's position
            if (board->debug_isOnMyMinPath[i][j])             { tiles[ti++] = '*'; continue; } // '*' if tile is part of my min path
            tiles[ti++] = ' ';
        }
    }
    
    ti = 0;
    hi = 0;
    vi = 0;

    LOG_INFO(DOM) << "        0     1     2     3     4     5     6     7     8   ";
    LOG_INFO(DOM) << "     ╔═════════════════════════════════════════════════════╗";

    for (int i = 0; i < 8; i++) // repeat 8 times
    {
        // row i (tiles, vert walls)
        ClearBuff();    
        sprintf(buff + strlen(buff), "   %d ║", i); // row index
        for (int j = 0; j < 8; j++)
        {
            sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
            sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
        }
        sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
        LOG_INFO(DOM) << buff;

        // row i horiz walls
        ClearBuff();    
        sprintf(buff + strlen(buff), "     ║ ");
        for (int j = 0; j < 8; j++)
        {
            sprintf(buff + strlen(buff), "%c%c%c - ", horizw[hi], horizw[hi], horizw[hi]);
            hi++;
        }
        sprintf(buff + strlen(buff), "%c%c%c ║", horizw[hi], horizw[hi], horizw[hi]);
        hi++;    
        LOG_INFO(DOM) << buff;
    }

    // last row (only tiles and vert walls)
    ClearBuff();    
    sprintf(buff + strlen(buff), "   %d ║", 8); // row index
    for (int j = 0; j < 8; j++)
    {
        sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
        sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
    }
    sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
    LOG_INFO(DOM) << buff;

    LOG_INFO(DOM) << "     ╚═════════════════════════════════════════════════════╝ ";
}
    