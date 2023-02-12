
#include <stddef.h>
#include <string.h>
#include "board.h"

static void DecreaseWallPermission(Wall_t* wall);
static void IncreaseWallPermission(Wall_t* wall);

#if (USE_POSSIBLE_WALLS_LIST)
static void RemoveFromPossibleWalls(Wall_t* wall);
static void AddToPossibleWalls(Wall_t* wall);
#endif

static Board_t board;


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
            board.tiles[x][y].north = ((x > 0)              ?   &(board.tiles[x - 1][y]) : NULL);
            board.tiles[x][y].south = ((x < (BOARD_SZ - 1)) ?   &(board.tiles[x + 1][y]) : NULL);
            board.tiles[x][y].west  = ((y > 0)              ?   &(board.tiles[x][y - 1]) : NULL);
            board.tiles[x][y].east  = ((y < (BOARD_SZ - 1)) ?   &(board.tiles[x][y + 1]) : NULL);

            // mark first row tiles as goal tiles for me, and last row tiles as goal tiles for opponent
            board.tiles[x][y].isGoalFor = ((x == 0) ? ME : ((x == (BOARD_SZ - 1)) ? OPPONENT : NONE));           
        }
    }

    // init walls
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {
                board.walls[o][x][y].pos = {x, y};
                board.walls[o][x][y].orientation = (Orientation_t)o;
                board.walls[o][x][y].permission = WALL_PERMITTED;

                // set tiles that this wall separates when placed
                Tile_t* referenceTile = &(board.tiles[x][y]);
                board.walls[o][x][y].northwest = referenceTile;
                board.walls[o][x][y].northeast = referenceTile->east;
                board.walls[o][x][y].southwest = referenceTile->south;
                board.walls[o][x][y].southeast = referenceTile->south->east;

                // link to walls that this wall forbids when placed
                if (o == H)
                {
                    // each horizontal wall forbids 2 other horizontal walls and a vertical wall
                    board.walls[o][x][y].forbidsPrev  = ((y == 0) ?  NULL : &(board.walls[H][x][y - 1]));
                    board.walls[o][x][y].forbidsNext  = ((y == (BOARD_SZ - 2)) ? NULL : &(board.walls[H][x][y + 1]));
                    board.walls[o][x][y].forbidsCompl = &(board.walls[V][x][y]);

                }
                else
                {
                    // each vertical wall forbids 2 other vertical walls and a horizontal wall
                    board.walls[o][x][y].forbidsPrev  = ((x == 0) ?     NULL : &(board.walls[V][x - 1][y]));
                    board.walls[o][x][y].forbidsNext  = ((x == (BOARD_SZ - 2)) ? NULL : &(board.walls[V][x + 1][y]));
                    board.walls[o][x][y].forbidsCompl = &(board.walls[H][x][y]);
                }

                // link to the next and prev possible walls (initially all walls are possible)
                board.walls[o][x][y].possiblePrev = ((x == 0 && y == 0) ? NULL // the first wall doesn't have a prev
                                                    : ((y == 0) ? &(board.walls[o][x - 1][BOARD_SZ - 2]) // first in col links to last in prev row
                                                    : &(board.walls[o][x][y - 1]))); // all others link to the prev in col

                board.walls[o][x][y].possibleNext = ((x == (BOARD_SZ - 2) && y == (BOARD_SZ - 2)) ? NULL // the last wall doesn't have a next
                                                    : (y == (BOARD_SZ - 2) ? &(board.walls[o][x + 1][0]) // last in col links to first in next row
                                                    : &(board.walls[o][x][y + 1]))); // all others link to the next in col
            }
        }
    }

    // link the last horiz wall to the first vert wall
    board.walls[H][BOARD_SZ - 2][BOARD_SZ - 2].possibleNext = &(board.walls[V][0][0]);
    board.walls[V][0][0].possiblePrev = &(board.walls[H][BOARD_SZ - 2][BOARD_SZ - 2]);

    // set the first horiz wall as possible walls list head
    board.firstPossibleWall =&(board.walls[H][0][0]);

    // init number of walls left
    board.wallsLeft[ME] = 10;
    board.wallsLeft[OPPONENT] = 10;

    // set initial player positions
    board.playerPos[ME] = {(BOARD_SZ - 1), BOARD_SZ / 2};
    board.playerPos[OPPONENT] = {0, BOARD_SZ / 2};

    // set other player for each player :)
    board.otherPlayer[ME] = OPPONENT;
    board.otherPlayer[OPPONENT] = ME;
}

Board_t* GetBoard(void)
{
    return &board;
}

Wall_t* GetWall(Position_t wallPos, Orientation_t wallOr)
{
    return &(board.walls[wallOr][wallPos.x][wallPos.y]);
}

void UpdatePos(Player_t player, Position_t pos)
{
    board.playerPos[player] = pos;
}

void UpdateWallsLeft(Player_t player, uint8_t wallsLeft)
{
    board.wallsLeft[player] = wallsLeft;
}

void PlaceWall(Player_t player, Wall_t* wall)
{
    // Placing a wall means:
    // 1. removing links (graph edges) between tiles separated by the wall
    if(wall->orientation == H)
    {
        wall->northwest->south = NULL;
        wall->northeast->south = NULL;
        wall->southwest->north = NULL;
        wall->southeast->north = NULL;
    }
    else
    {
        wall->northwest->east = NULL;
        wall->northeast->west = NULL;
        wall->southwest->east = NULL;
        wall->southeast->west = NULL;
    }

    // 2. Remove the wall, along with the walls it displaces, from the possible walls list.
    // Only walls that are permitted will be removed (to avoid double removal).
    #if (USE_POSSIBLE_WALLS_LIST)
    RemoveFromPossibleWalls(wall);
    RemoveFromPossibleWalls(wall->forbidsPrev);
    RemoveFromPossibleWalls(wall->forbidsNext);
    RemoveFromPossibleWalls(wall->forbidsCompl);
    #endif

    // 3. Forbidding the given wall, along with the walls it displaces, from future use.
    // Any decrease in the permission level of a wall renders that wall forbidden.
    // Multiple levels of permission are needed because a wall can be forbidden by 1 or more walls.
    DecreaseWallPermission(wall);
    DecreaseWallPermission(wall->forbidsPrev);
    DecreaseWallPermission(wall->forbidsNext);
    DecreaseWallPermission(wall->forbidsCompl);


    // 4. Adjusting number of walls left for given player
    board.wallsLeft[player]--;
}

void UndoWall(Player_t player, Wall_t* wall)
{
    // Undoing a wall means:
    // 1. Restoring links (graph edges) between tiles separated by the wall
    if(wall->orientation == H)
    {
        wall->northwest->south = wall->southwest;
        wall->northeast->south = wall->southeast;
        wall->southwest->north = wall->northwest;
        wall->southeast->north = wall->northeast;
    }
    else
    {
        wall->northwest->east = wall->northeast;
        wall->northeast->west = wall->northwest;
        wall->southwest->east = wall->southeast;
        wall->southeast->west = wall->southwest;
    }

    // 2. Increasing the permissions for the current wall and the walls it is displacing.
    // A wall can be forbidden by 1 or more walls, so an increase in the permission level doesn't necessarily mean it is permitted.
    // A wall will be permitted only when its permission level is max.
    IncreaseWallPermission(wall->forbidsCompl);
    IncreaseWallPermission(wall->forbidsNext);
    IncreaseWallPermission(wall->forbidsPrev);
    IncreaseWallPermission(wall);

    // 3. Adding the wall back to the possible walls list (only if the wall just became permitted).
    // Walls will be added back in the reverse order of the removal.
    #if (USE_POSSIBLE_WALLS_LIST)
    AddToPossibleWalls(wall->forbidsCompl);
    AddToPossibleWalls(wall->forbidsNext);
    AddToPossibleWalls(wall->forbidsPrev);
    AddToPossibleWalls(wall);
    #endif

    // 4. Adjusting number of walls left for given player
    board.wallsLeft[player]++;
}


void UpdatePossibleMoves(Player_t player)
{
    Tile_t* pT = GetPlayerTile(player);
    Tile_t* oT = GetPlayerTile(board.otherPlayer[player]);

    // set all moves to NOT POSSIBLE
    memset(board.moves[player], 0, sizeof(board.moves[player]));

    // set possible moves
    board.moves[player][MOVE_NORTH].isPossible = ((pT->north) && (pT->north != oT) ? true : false);
    board.moves[player][MOVE_SOUTH].isPossible = ((pT->south) && (pT->south != oT) ? true : false);
    board.moves[player][MOVE_EAST].isPossible = ((pT->east) && (pT->east != oT) ? true : false);
    board.moves[player][MOVE_WEST].isPossible = ((pT->west) && (pT->west != oT) ? true : false);

    board.moves[player][JUMP_NORTH].isPossible = ((pT->north) && (pT->north == oT) && (oT->north) ? true : false);
    board.moves[player][JUMP_SOUTH].isPossible = ((pT->south) && (pT->south == oT) && (oT->south) ? true : false);
    board.moves[player][JUMP_EAST].isPossible = ((pT->east) && (pT->east == oT) && (oT->east) ? true : false);
    board.moves[player][JUMP_WEST].isPossible = ((pT->west) && (pT->west == oT) && (oT->west) ? true : false);

    board.moves[player][JUMP_NORTH_EAST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->east)) ? true :          // jump N -> E
                                                        (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->north)) ? true : false)); // jump E -> N
    board.moves[player][JUMP_NORTH_WEST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->west)) ? true :          // jump N -> W
                                                        (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->north)) ? true : false)); // jump W -> N                          
    board.moves[player][JUMP_SOUTH_EAST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->east)) ? true :          // jump S -> E
                                                        (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->south)) ? true : false)); // jump E -> S
    board.moves[player][JUMP_SOUTH_WEST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->west)) ? true :          // jump S -> W
                                                        (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->south)) ? true : false)); // jump W -> S
}

#if (USE_POSSIBLE_WALLS_LIST)
static void RemoveFromPossibleWalls(Wall_t* wall)
{
    if (wall && (wall->permission == WALL_PERMITTED)) // only remove if it hasn't been removed before
    {
        if (board.firstPossibleWall == wall)
        {
            // head to be removed so just update head
            board.firstPossibleWall = (board.firstPossibleWall)->possibleNext;
            (board.firstPossibleWall)->possiblePrev = NULL;
        }
        else
        {
            // bypass forward and backwards
            wall->possiblePrev->possibleNext = wall->possibleNext;
            if (wall->possibleNext)
            {
                wall->possibleNext->possiblePrev = wall->possiblePrev;
            }
        }
    }
}

static void AddToPossibleWalls(Wall_t* wall)
{
    if (wall && (wall->permission == WALL_PERMITTED)) // only add if wall is possible again
    {
        // if the item was head before removal, it points to the current head
        if (wall->possibleNext == board.firstPossibleWall)
        {
            // undo head moving
            board.firstPossibleWall->possiblePrev = wall;
            board.firstPossibleWall = wall;
        }
        else
        {
            // relink wall
            if (wall->possibleNext)
            {
                wall->possibleNext->possiblePrev = wall;
            }
            if (wall->possiblePrev)
            {
                wall->possiblePrev->possibleNext = wall;
            }
        }
    }
}
#endif

static void IncreaseWallPermission(Wall_t* wall)
{
    if (wall)
    {
        wall->permission = (WallPermission_t)(wall->permission + 1);   
    }
}

static void DecreaseWallPermission(Wall_t* wall)
{
    if (wall)
    {
        wall->permission = (WallPermission_t)(wall->permission - 1);        
    }
}

Tile_t* GetPlayerTile(Player_t player)
{
    return (&(board.tiles[board.playerPos[player].x][board.playerPos[player].y]));
}
