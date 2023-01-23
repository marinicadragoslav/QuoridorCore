
#include <stddef.h>
#include "board.h"

#define L (BOARD_SZ - 1)
#define WALL_POSSIBLE 1U

static void PlaceHWall(Position_t pos);
static void PlaceVWall(Position_t pos);
static void RenderImpossibleHWall(HWall_t* wall);
static void RenderImpossibleVWall(VWall_t* wall);
static void UndoHWall(Position_t pos);
static void UndoVWall(Position_t pos);
static void RenderPossibleHWall(HWall_t* wall);
static void RenderPossibleVWall(VWall_t* wall);
static void RemoveFromPossibleHWallsList(HWall_t* wall);
static void RemoveFromPossibleVWallsList(VWall_t* wall);
static void AddToPossibleHWallsList(HWall_t* wall);
static void AddToPossibleVWallsList(VWall_t* wall);

static Board_t board;

Board_t* GetBoard(void)
{
    return &board;
}

void InitBoard(void)
{
    // init tiles
    for (int8_t x = 0; x < BOARD_SZ; x++)
    {
        for (int8_t y = 0; y < BOARD_SZ; y++)
        {
            // set current tile's position
            board.tiles[x][y].pos = {x, y};

            // link tile to its neighbours. Link to NULL if the tile is on the border.
            board.tiles[x][y].north = ((x > 0) ? &(board.tiles[x - 1][y]) : NULL);
            board.tiles[x][y].south = ((x < L) ? &(board.tiles[x + 1][y]) : NULL);
            board.tiles[x][y].west  = ((y > 0) ? &(board.tiles[x][y - 1]) : NULL);
            board.tiles[x][y].east  = ((y < L) ? &(board.tiles[x][y + 1]) : NULL);

            // mark first row tiles as goal tiles for me, and last row tiles as goal tiles for opponent
            board.tiles[x][y].isGoalForMe =  ((x == 0) ? true : false);
            board.tiles[x][y].isGoalForOpp = ((x == L) ? true : false);           
        }
    }

    // init horizontal walls
    for (int8_t x = 0; x < L; x++)
    {
        for (int8_t y = 0; y < L; y++)
        {
            board.hWalls[x][y].pos = {x, y};

            // wall is possible
            board.hWalls[x][y].possibleFlag = WALL_POSSIBLE;
            board.hWalls[x][y].debug_isPossible = true;

            /*
            // link hWalls doubly-linked style
            
            board.hWalls[x][y].prev = ((x == 0 && y == 0) ? NULL : ((y == 0) ? &(board.hWalls[x - 1][L]) : &(board.hWalls[x][y - 1])));
            board.hWalls[x][y].next = ((x == L && y == L) ? NULL : ((y == L) ? &(board.hWalls[x + 1][0]) : &(board.hWalls[x][y + 1])));
            */
            // link to neighbour walls
            board.hWalls[x][y].west = ((y == 0) ?     NULL : &(board.hWalls[x][y - 1]));
            board.hWalls[x][y].east = ((y == L - 1) ? NULL : &(board.hWalls[x][y + 1]));

            // set tiles that this wall separates when placed
            board.hWalls[x][y].northwest = &(board.tiles[x][y]);
            board.hWalls[x][y].northeast = board.tiles[x][y].east;
            board.hWalls[x][y].southwest = board.tiles[x][y].south;
            board.hWalls[x][y].southeast = (board.tiles[x][y].south)->east;
        }
    }

    // init vertical walls
    for (int8_t x = 0; x < L; x++)
    {
        for (int8_t y = 0; y < L; y++)
        {
            board.vWalls[x][y].pos = {x, y};

            // wall is possible
            board.vWalls[x][y].possibleFlag = WALL_POSSIBLE;
            board.vWalls[x][y].debug_isPossible = true;

            /*
            // link vWalls doubly-linked style
            board.vWalls[x][y].prev = ((x == 0 && y == 0) ? NULL : ((y == 0) ? &(board.vWalls[x - 1][L]) : &(board.vWalls[x][y - 1])));
            board.vWalls[x][y].next = ((x == L && y == L) ? NULL : ((y == L) ? &(board.vWalls[x + 1][0]) : &(board.vWalls[x][y + 1])));
            */
            // link to neighbour walls
            board.vWalls[x][y].north = ((x == 0) ?     NULL : &(board.vWalls[x - 1][y]));
            board.vWalls[x][y].south = ((x == L - 1) ? NULL : &(board.vWalls[x + 1][y]));

            // set tiles that this wall separates when placed
            board.vWalls[x][y].northwest = &(board.tiles[x][y]);
            board.vWalls[x][y].northeast = board.tiles[x][y].east;
            board.vWalls[x][y].southwest = board.tiles[x][y].south;
            board.vWalls[x][y].southeast = (board.tiles[x][y].south)->east;
        }
    }

    // init linked list heads
    board.hWallFirst = &(board.hWalls[0][0]);
    board.vWallFirst = &(board.vWalls[0][0]);
    board.moveFirst = &(board.moves[0]);

    // init number of walls left
    board.myWallsLeft = 10;
    board.oppWallsLeft = 10;

    // set initial player positions
    board.myPos =  {L, BOARD_SZ / 2};
    board.oppPos = {0, BOARD_SZ / 2};
}

void UpdateMyPos(Position_t pos)
{
    board.myPos = pos;
}

void UpdateOpponentPos(Position_t pos)
{
    board.oppPos = pos;
}

void UpdateMyWallsLeft(uint8_t n)
{
    board.myWallsLeft = n;
}

void UpdateOpponentWallsLeft(uint8_t n)
{
    board.oppWallsLeft = n;
}

void PlaceHWallByMe(Position_t pos)
{
    PlaceHWall(pos);
    board.myWallsLeft--;
}

void PlaceHWallByOpponent(Position_t pos)
{
    PlaceHWall(pos);
    board.oppWallsLeft--;
}

void PlaceVWallByMe(Position_t pos)
{
    PlaceVWall(pos);
    board.myWallsLeft--;
}

void PlaceVWallByOpponent(Position_t pos)
{
    PlaceVWall(pos);
    board.oppWallsLeft--;
}

void UndoHWallByMe(Position_t pos)
{
    UndoHWall(pos);
    board.myWallsLeft++;
}

void UndoHWallByOpponent(Position_t pos)
{
    UndoHWall(pos);
    board.oppWallsLeft++;
}

void UndoVWallByMe(Position_t pos)
{
    UndoVWall(pos);
    board.myWallsLeft++;
}

void UndoVWallByOpponent(Position_t pos)
{
    UndoVWall(pos);
    board.oppWallsLeft++;
}

static void PlaceHWall(Position_t pos)
{
    HWall_t* wall = &(board.hWalls[pos.x][pos.y]);

    RenderImpossibleHWall(wall);

    // each H wall renders impossible 3 other walls (the 2 H neighbours and one V wall)
    RenderImpossibleHWall(wall->west);
    RenderImpossibleHWall(wall->east);
    RenderImpossibleVWall(&(board.vWalls[pos.x][pos.y]));

    // remove links between tiles separated by the wall
    wall->northwest->south = NULL;
    wall->northeast->south = NULL;
    wall->southwest->north = NULL;
    wall->southeast->north = NULL;
}

static void PlaceVWall(Position_t pos)
{
    VWall_t* wall = &(board.vWalls[pos.x][pos.y]);

    RenderImpossibleVWall(wall);

    // each V wall renders impossible 3 other walls (the 2 V neighbours and one H wall)
    RenderImpossibleVWall(wall->north);
    RenderImpossibleVWall(wall->south);
    RenderImpossibleHWall(&(board.hWalls[pos.x][pos.y]));

    // remove links between tiles separated by the wall
    wall->northwest->east = NULL;
    wall->northeast->west = NULL;
    wall->southwest->east = NULL;
    wall->southeast->west = NULL;
}

static void RenderImpossibleHWall(HWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag <<= 1U;
        RemoveFromPossibleHWallsList(wall);
    }
}

static void RenderImpossibleVWall(VWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag <<= 1U;
        RemoveFromPossibleVWallsList(wall);
    }
}

static void UndoHWall(Position_t pos)
{
    HWall_t* wall = &(board.hWalls[pos.x][pos.y]);

    RenderPossibleHWall(wall);

    // revert constraints on the 2 H neighbours and one V wall
    RenderPossibleHWall(wall->west);
    RenderPossibleHWall(wall->east);
    RenderPossibleVWall(&(board.vWalls[pos.x][pos.y]));

    // restore links between tiles that were separated by the wall
    wall->northwest->south = wall->southwest;
    wall->northeast->south = wall->southeast;
    wall->southwest->north = wall->northwest;
    wall->southeast->north = wall->northeast;
}

static void UndoVWall(Position_t pos)
{
    VWall_t* wall = &(board.vWalls[pos.x][pos.y]);

    RenderPossibleVWall(wall);

    // revert constraints on the 2 V neighbours and one H wall
    RenderPossibleVWall(wall->north);
    RenderPossibleVWall(wall->south);
    RenderPossibleHWall(&(board.hWalls[pos.x][pos.y]));

    // restore links between tiles that were separated by the wall
    wall->northwest->east = wall->northeast;
    wall->northeast->west = wall->northwest;
    wall->southwest->east = wall->southeast;
    wall->southeast->west = wall->southwest;
}

static void RenderPossibleHWall(HWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag >>= 1U;

        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            AddToPossibleHWallsList(wall);
        }
    }
}

static void RenderPossibleVWall(VWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag >>= 1U;

        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            AddToPossibleVWallsList(wall);
        }
    }
}

static void RemoveFromPossibleHWallsList(HWall_t* wall)
{
    wall->debug_isPossible = false;
}

static void RemoveFromPossibleVWallsList(VWall_t* wall)
{
    wall->debug_isPossible = false;
}

static void AddToPossibleHWallsList(HWall_t* wall)
{
    wall->debug_isPossible = true;
}

static void AddToPossibleVWallsList(VWall_t* wall)
{
    wall->debug_isPossible = true;
}

