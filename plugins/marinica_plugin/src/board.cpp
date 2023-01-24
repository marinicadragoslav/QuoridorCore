
#include <stddef.h>
#include "board.h"

#define L (BOARD_SZ - 1)
#define WALL_POSSIBLE 1U

static void PlaceHorizWall(Position_t pos);
static void PlaceVertWall(Position_t pos);
static void ForbidHorizWall(HWall_t* wall);
static void ForbidVertWall(VWall_t* wall);
static void UndoHorizWall(Position_t pos);
static void UndoVertWall(Position_t pos);
static void AllowHorizWall(HWall_t* wall);
static void AllowVertWall(VWall_t* wall);
static void RemoveFromPossibleHorizWallsList(HWall_t* wall);
static void RemoveFromPossibleVertWallsList(VWall_t* wall);
static void AddToPossibleHorizWallsList(HWall_t* wall);
static void AddToPossibleVertWallsList(VWall_t* wall);

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

            // link to neighbour walls
            board.hWalls[x][y].west = ((y == 0) ?     NULL : &(board.hWalls[x][y - 1]));
            board.hWalls[x][y].east = ((y == L - 1) ? NULL : &(board.hWalls[x][y + 1]));

            // set tiles that this wall separates when placed
            board.hWalls[x][y].northwest = &(board.tiles[x][y]);
            board.hWalls[x][y].northeast = board.tiles[x][y].east;
            board.hWalls[x][y].southwest = board.tiles[x][y].south;
            board.hWalls[x][y].southeast = (board.tiles[x][y].south)->east;

            // add to possible horizontal walls list
            int8_t i = x * L + y;
            board.possibleHorizWallsList[i].wall = &(board.hWalls[x][y]);

            // link list item
            board.possibleHorizWallsList[i].prev = ((x == 0 && y == 0) ? NULL : &(board.possibleHorizWallsList[i - 1]));
            board.possibleHorizWallsList[i].next = ((x == L - 1 && y == L - 1) ? NULL : &(board.possibleHorizWallsList[i + 1]));
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

            // link to neighbour walls
            board.vWalls[x][y].north = ((x == 0) ?     NULL : &(board.vWalls[x - 1][y]));
            board.vWalls[x][y].south = ((x == L - 1) ? NULL : &(board.vWalls[x + 1][y]));

            // set tiles that this wall separates when placed
            board.vWalls[x][y].northwest = &(board.tiles[x][y]);
            board.vWalls[x][y].northeast = board.tiles[x][y].east;
            board.vWalls[x][y].southwest = board.tiles[x][y].south;
            board.vWalls[x][y].southeast = (board.tiles[x][y].south)->east;

            // add to possible vertical walls list
            int8_t i = x * L + y;
            board.possibleVertWallsList[i].wall = &(board.vWalls[x][y]);

            // link list item
            board.possibleVertWallsList[i].prev = ((x == 0 && y == 0) ? NULL : &(board.possibleVertWallsList[i - 1]));
            board.possibleVertWallsList[i].next = ((x == L - 1 && y == L - 1) ? NULL : &(board.possibleVertWallsList[i + 1]));
        }
    }

    // init moves
    for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
    {
        board.moves[i] = (Move_t)i;

        // add to possible moves list
        board.possibleMovesList[i].move = &(board.moves[i]);

        // link list item
        board.possibleMovesList[i].prev = (i == MOVE_FIRST ? NULL : &(board.possibleMovesList[i - 1]));
        board.possibleMovesList[i].next = (i == MOVE_LAST ? NULL : &(board.possibleMovesList[i - 1]));
    }

    // init linked list heads
    board.headPHWL = &(board.possibleHorizWallsList[0]);
    board.headPVWL = &(board.possibleVertWallsList[0]);
    board.headPML = &(board.possibleMovesList[MOVE_FIRST]);

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

void PlaceHorizWallByMe(Position_t pos)
{
    PlaceHorizWall(pos);
    board.myWallsLeft--;
}

void PlaceHorizWallByOpponent(Position_t pos)
{
    PlaceHorizWall(pos);
    board.oppWallsLeft--;
}

void PlaceVertWallByMe(Position_t pos)
{
    PlaceVertWall(pos);
    board.myWallsLeft--;
}

void PlaceVertWallByOpponent(Position_t pos)
{
    PlaceVertWall(pos);
    board.oppWallsLeft--;
}

void UndoHorizWallByMe(Position_t pos)
{
    UndoHorizWall(pos);
    board.myWallsLeft++;
}

void UndoHorizWallByOpponent(Position_t pos)
{
    UndoHorizWall(pos);
    board.oppWallsLeft++;
}

void UndoVertWallByMe(Position_t pos)
{
    UndoVertWall(pos);
    board.myWallsLeft++;
}

void UndoVertWallByOpponent(Position_t pos)
{
    UndoVertWall(pos);
    board.oppWallsLeft++;
}

static void PlaceHorizWall(Position_t pos)
{
    HWall_t* wall = &(board.hWalls[pos.x][pos.y]);

    ForbidHorizWall(wall);

    // each H wall renders impossible 3 other walls (the 2 H neighbours and one V wall)
    ForbidHorizWall(wall->west);
    ForbidHorizWall(wall->east);
    ForbidVertWall(&(board.vWalls[pos.x][pos.y]));

    // remove links between tiles separated by the wall
    wall->northwest->south = NULL;
    wall->northeast->south = NULL;
    wall->southwest->north = NULL;
    wall->southeast->north = NULL;
}

static void PlaceVertWall(Position_t pos)
{
    VWall_t* wall = &(board.vWalls[pos.x][pos.y]);

    ForbidVertWall(wall);

    // each V wall renders impossible 3 other walls (the 2 V neighbours and one H wall)
    ForbidVertWall(wall->north);
    ForbidVertWall(wall->south);
    ForbidHorizWall(&(board.hWalls[pos.x][pos.y]));

    // remove links between tiles separated by the wall
    wall->northwest->east = NULL;
    wall->northeast->west = NULL;
    wall->southwest->east = NULL;
    wall->southeast->west = NULL;
}

static void ForbidHorizWall(HWall_t* wall)
{
    if (wall)
    {
        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            RemoveFromPossibleHorizWallsList(wall);
        }

        wall->possibleFlag <<= 1U;        
    }
}

static void ForbidVertWall(VWall_t* wall)
{
    if (wall)
    {
        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            RemoveFromPossibleVertWallsList(wall);
        }

        wall->possibleFlag <<= 1U;
    }
}

static void UndoHorizWall(Position_t pos)
{
    HWall_t* wall = &(board.hWalls[pos.x][pos.y]);

    AllowHorizWall(wall);

    // revert constraints on the 2 H neighbours and one V wall
    AllowHorizWall(wall->west);
    AllowHorizWall(wall->east);
    AllowVertWall(&(board.vWalls[pos.x][pos.y]));

    // restore links between tiles that were separated by the wall
    wall->northwest->south = wall->southwest;
    wall->northeast->south = wall->southeast;
    wall->southwest->north = wall->northwest;
    wall->southeast->north = wall->northeast;
}

static void UndoVertWall(Position_t pos)
{
    VWall_t* wall = &(board.vWalls[pos.x][pos.y]);

    AllowVertWall(wall);

    // revert constraints on the 2 V neighbours and one H wall
    AllowVertWall(wall->north);
    AllowVertWall(wall->south);
    AllowHorizWall(&(board.hWalls[pos.x][pos.y]));

    // restore links between tiles that were separated by the wall
    wall->northwest->east = wall->northeast;
    wall->northeast->west = wall->northwest;
    wall->southwest->east = wall->southeast;
    wall->southeast->west = wall->southwest;
}

static void AllowHorizWall(HWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag >>= 1U;

        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            AddToPossibleHorizWallsList(wall);
        }
    }
}

static void AllowVertWall(VWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag >>= 1U;

        if (wall->possibleFlag == WALL_POSSIBLE)
        {
            AddToPossibleVertWallsList(wall);
        }
    }
}

static void RemoveFromPossibleHorizWallsList(HWall_t* wall)
{
    if ((board.headPHWL)->wall == wall)
    {
        // head to be removed so just update head
        board.headPHWL = (board.headPHWL)->next;
        (board.headPHWL)->prev = NULL;
    }
    else
    {
        // find item
        int8_t index = wall->pos.x * L + wall->pos.y;
        HorizWallsListItem_t* item = &(board.possibleHorizWallsList[index]);

        // bypass item
        item->prev->next = item->next;
        if (item->next)
        {
            item->next->prev = item->prev;
        }
    }
}

static void RemoveFromPossibleVertWallsList(VWall_t* wall)
{
    if ((board.headPVWL)->wall == wall)
    {
        // head to be removed so just update head
        board.headPVWL = (board.headPVWL)->next;
        (board.headPVWL)->prev = NULL;
    }
    else
    {
        // find item
        int8_t index = wall->pos.x * L + wall->pos.y;
        VertWallsListItem_t* item = &(board.possibleVertWallsList[index]);

        // bypass item
        item->prev->next = item->next;
        if (item->next)
        {
            item->next->prev = item->prev;
        }
    }
}

static void AddToPossibleHorizWallsList(HWall_t* wall)
{
    // locate list item to add
    int8_t index = wall->pos.x * L + wall->pos.y;
    HorizWallsListItem_t* item = &(board.possibleHorizWallsList[index]);

    // add item in front of the list
    item->prev = NULL;
    item->next = board.headPHWL;
    (board.headPHWL)->prev = item;

    // update head
    board.headPHWL = item;
}

static void AddToPossibleVertWallsList(VWall_t* wall)
{
    // locate list item to add
    int8_t index = wall->pos.x * L + wall->pos.y;
    VertWallsListItem_t* item = &(board.possibleVertWallsList[index]);

    // add item in front of the list
    item->prev = NULL;
    item->next = board.headPVWL;
    (board.headPVWL)->prev = item;

    // update head
    board.headPVWL = item;
}

