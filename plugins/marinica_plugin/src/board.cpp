
#include <stddef.h>
#include <string.h>
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
static RelativePlayerPos_t GetPlayersRelativePos(void);

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
            board.hWalls[x][y].possibleFlag = WALL_POSSIBLE;

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
            board.vWalls[x][y].possibleFlag = WALL_POSSIBLE;

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

    // init number of walls left
    board.myWallsLeft = 10;
    board.oppWallsLeft = 10;

    // set initial player positions
    board.myPos =  {L, BOARD_SZ / 2};
    board.oppPos = {0, BOARD_SZ / 2};
}

bool IsMyPos(Position_t pos)
{
    return ((pos.x == board.myPos.x) && (pos.y == board.myPos.y));
}

bool IsOpponentsPos(Position_t pos)
{
    return ((pos.x == board.oppPos.x) && (pos.y == board.oppPos.y));
}

void UpdateMyPos(Position_t pos)
{
    board.myPos = pos;
}

void UpdateOpponentsPos(Position_t pos)
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

void UpdateMyPossibleMoves(void)  // optimize this FFS! maybe
{
    // first, set all moves to NOT POSSIBLE
    memset(board.myMoves, 0, sizeof(board.myMoves));

    // get my tile & opponent's tile
    Tile_t* myTile = &(board.tiles[board.myPos.x][board.myPos.y]);   
    Tile_t* oppTile = &(board.tiles[board.oppPos.x][board.oppPos.y]);      

    switch (GetPlayersRelativePos())
    {
        case NOT_FACE_TO_FACE: // most often                       
            // no jumps possible, so only worry about one-step-moves:               
            board.myMoves[MOVE_NORTH].isPossible = (myTile->north ? true : false);
            board.myMoves[MOVE_SOUTH].isPossible = (myTile->south ? true : false);
            board.myMoves[MOVE_EAST].isPossible = (myTile->east ? true : false);
            board.myMoves[MOVE_WEST].isPossible = (myTile->west ? true : false);
        break;

        case OPPONENT_ABOVE_ME:
            // moving one step north not possible, check for jumping:
            if (myTile->north) // no wall above me
            {
                if (oppTile->north) // no wall or border above him
                {
                    board.myMoves[JUMP_NORTH].isPossible = true; // allow jumping straight over
                }
                else // wall or border above him
                {
                    if (oppTile->west) // no wall or border to his left
                    {
                        board.myMoves[JUMP_NORTH_WEST].isPossible = true;
                    }
                    if (oppTile->east) // no wall or border to his right
                    {
                        board.myMoves[JUMP_NORTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.myMoves[MOVE_SOUTH].isPossible = (myTile->south ? true : false);
            board.myMoves[MOVE_EAST].isPossible = (myTile->east ? true : false);
            board.myMoves[MOVE_WEST].isPossible = (myTile->west ? true : false);
        break;

        case OPPONENT_BELOW_ME:
            // moving one step south not possible, check for jumping:
            if (myTile->south) // no wall below me
            {
                if (oppTile->south) // no wall or border below him
                {
                    board.myMoves[JUMP_SOUTH].isPossible = true; // allow jumping straight over
                }
                else // wall or border below him
                {
                    if (oppTile->west) // no wall or border to his left
                    {
                        board.myMoves[JUMP_SOUTH_WEST].isPossible = true;
                    }
                    if (oppTile->east) // no wall or border to his right
                    {
                        board.myMoves[JUMP_SOUTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.myMoves[MOVE_NORTH].isPossible = (myTile->north ? true : false);
            board.myMoves[MOVE_EAST].isPossible = (myTile->east ? true : false);
            board.myMoves[MOVE_WEST].isPossible = (myTile->west ? true : false);
        break;

        case OPPONENT_TO_MY_LEFT:
            // moving one step west not possible, check for jumping:
            if (myTile->west) // no wall to my left
            {
                if (oppTile->west) // no wall or border to his left
                {
                    board.myMoves[JUMP_WEST].isPossible = true; // allow jumping straight over
                }
                else // wall or border to his left
                {
                    if (oppTile->north) // no wall or border above him
                    {
                        board.myMoves[JUMP_NORTH_WEST].isPossible = true;
                    }
                    if (oppTile->south) // no wall or border below him
                    {
                        board.myMoves[JUMP_SOUTH_WEST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.myMoves[MOVE_NORTH].isPossible = (myTile->north ? true : false);
            board.myMoves[MOVE_SOUTH].isPossible = (myTile->south ? true : false);
            board.myMoves[MOVE_EAST].isPossible = (myTile->east ? true : false);
        break;

        case OPPONENT_TO_MY_RIGHT:
            // moving one step east not possible, check for jumping:
            if (myTile->east) // no wall to my right
            {
                if (oppTile->east) // no wall or border to his right
                {
                    board.myMoves[JUMP_EAST].isPossible = true; // allow jumping straight over
                }
                else // wall or border to his right
                {
                    if (oppTile->north) // no wall or border above him
                    {
                        board.myMoves[JUMP_NORTH_EAST].isPossible = true;
                    }
                    if (oppTile->south) // no wall or border below him
                    {
                        board.myMoves[JUMP_SOUTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.myMoves[MOVE_NORTH].isPossible = (myTile->north ? true : false);
            board.myMoves[MOVE_SOUTH].isPossible = (myTile->south ? true : false);
            board.myMoves[MOVE_WEST].isPossible = (myTile->west ? true : false);
        break;

        default: // this can't happen
        break;
    }
}

static RelativePlayerPos_t GetPlayersRelativePos(void)
{
    if (board.myPos.x == board.oppPos.x)
    {
        if (board.myPos.y == board.oppPos.y - 1)
        {
            return OPPONENT_TO_MY_RIGHT;
        }
        if (board.myPos.y == board.oppPos.y + 1)
        {
            return OPPONENT_TO_MY_LEFT;
        }
    }

    if (board.myPos.y == board.oppPos.y)
    {
        if (board.myPos.x == board.oppPos.x - 1)
        {
            return OPPONENT_BELOW_ME;
        }
        if (board.myPos.x == board.oppPos.x + 1)
        {
            return OPPONENT_ABOVE_ME;
        }
    }

    return NOT_FACE_TO_FACE;
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
        wall->possibleFlag <<= 1U;        
    }
}

static void ForbidVertWall(VWall_t* wall)
{
    if (wall)
    {
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
    }
}

static void AllowVertWall(VWall_t* wall)
{
    if (wall)
    {
        wall->possibleFlag >>= 1U;
    }
}