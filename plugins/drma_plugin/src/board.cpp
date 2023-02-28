
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "board.h"

namespace qplugin_drma {

static void DecreaseWallPermission(Wall_t* wall);
static void IncreaseWallPermission(Wall_t* wall);


Board_t* NewDefaultBoard(void)
{
    Board_t* board = (Board_t*)calloc(1, sizeof(Board_t));

    // init tiles
    for (int8_t x = 0; x < BOARD_SZ; x++)
    {
        for (int8_t y = 0; y < BOARD_SZ; y++)
        {
            // set current tile's position
            board->tiles[x][y].pos = {x, y};

            // link tile to its neighbours. Link to NULL if the tile is on the border.
            board->tiles[x][y].north = ((x > 0)              ?   &(board->tiles[x - 1][y]) : NULL);
            board->tiles[x][y].south = ((x < (BOARD_SZ - 1)) ?   &(board->tiles[x + 1][y]) : NULL);
            board->tiles[x][y].west  = ((y > 0)              ?   &(board->tiles[x][y - 1]) : NULL);
            board->tiles[x][y].east  = ((y < (BOARD_SZ - 1)) ?   &(board->tiles[x][y + 1]) : NULL);

            // mark first row tiles as goal tiles for me, and last row tiles as goal tiles for opponent
            board->tiles[x][y].isGoalFor = ((x == 0) ? ME : ((x == (BOARD_SZ - 1)) ? OPPONENT : NOBODY));           
        }
    }

    // init walls
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {
                board->walls[o][x][y].pos = {x, y};
                board->walls[o][x][y].orientation = (Orientation_t)o;
                board->walls[o][x][y].permission = WALL_PERMITTED;
                board->walls[o][x][y].isDisabled = false;

                // set tiles that this wall separates when placed
                Tile_t* referenceTile = &(board->tiles[x][y]);
                board->walls[o][x][y].northwest = referenceTile;
                board->walls[o][x][y].northeast = referenceTile->east;
                board->walls[o][x][y].southwest = referenceTile->south;
                board->walls[o][x][y].southeast = referenceTile->south->east;

                // link to walls that this wall forbids when placed
                if (o == H)
                {
                    // each horizontal wall forbids 2 other horizontal walls and a vertical wall
                    board->walls[o][x][y].forbidsPrev  = ((y == 0) ?  NULL : &(board->walls[H][x][y - 1]));
                    board->walls[o][x][y].forbidsNext  = ((y == (BOARD_SZ - 2)) ? NULL : &(board->walls[H][x][y + 1]));
                    board->walls[o][x][y].forbidsCompl = &(board->walls[V][x][y]);
                }
                else
                {
                    // each vertical wall forbids 2 other vertical walls and a horizontal wall
                    board->walls[o][x][y].forbidsPrev  = ((x == 0) ?     NULL : &(board->walls[V][x - 1][y]));
                    board->walls[o][x][y].forbidsNext  = ((x == (BOARD_SZ - 2)) ? NULL : &(board->walls[V][x + 1][y]));
                    board->walls[o][x][y].forbidsCompl = &(board->walls[H][x][y]);
                }                
            }
        }
    }

    // init number of walls left
    board->wallsLeft[ME] = 10;
    board->wallsLeft[OPPONENT] = 10;

    // set initial player positions
    board->playerPos[ME] = {(BOARD_SZ - 1), BOARD_SZ / 2};
    board->playerPos[OPPONENT] = {0, BOARD_SZ / 2};

    // set other player for each player :)
    board->otherPlayer[ME] = OPPONENT;
    board->otherPlayer[OPPONENT] = ME;

    // set the difference on x and y axes that each move implies
    for (int moveID = MOVE_FIRST; moveID <= MOVE_LAST; moveID++)
    {
        switch (moveID)
        {
            case MOVE_NORTH:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
            break;
            case MOVE_SOUTH:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
            break;
            case MOVE_WEST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
            break;
            case MOVE_EAST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
            break;
            case JUMP_NORTH:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -2;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
            break;
            case JUMP_SOUTH:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +2;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff =  0;
            break;
            case JUMP_WEST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -2;
            break;
            case JUMP_EAST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff =  0;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +2;
            break;
            case JUMP_NORTH_WEST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
            break;
            case JUMP_NORTH_EAST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = -1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
            break;
            case JUMP_SOUTH_WEST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = -1;
            break;
            case JUMP_SOUTH_EAST:
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = +1;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = +1;
            break;
            default: // this should never happen
                board->moves[ME][moveID].xDiff = board->moves[OPPONENT][moveID].xDiff = 0;
                board->moves[ME][moveID].yDiff = board->moves[OPPONENT][moveID].yDiff = 0;
        }
    }

    return board;
}

Wall_t* GetWallByPosAndOrientation(Board_t* board, Position_t wallPos, Orientation_t wallOr)
{
    return &(board->walls[wallOr][wallPos.x][wallPos.y]);
}

void UpdatePos(Board_t* board, Player_t player, Position_t pos)
{
    board->playerPos[player] = pos;
}

void UpdateWallsLeft(Board_t* board, Player_t player, uint8_t wallsLeft)
{
    board->wallsLeft[player] = wallsLeft;
}

void PlaceWall(Board_t* board, Player_t player, Wall_t* wall)
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

    // 2. Forbidding the given wall, along with the walls it displaces, from future use.
    // Any decrease in the permission level of a wall renders that wall forbidden.
    // Multiple levels of permission are needed because a wall can be forbidden by 1 or more walls.
    DecreaseWallPermission(wall);
    DecreaseWallPermission(wall->forbidsPrev);
    DecreaseWallPermission(wall->forbidsNext);
    DecreaseWallPermission(wall->forbidsCompl);

    // 3. Adjusting number of walls left for given player
    board->wallsLeft[player]--;
}

void UndoWall(Board_t* board, Player_t player, Wall_t* wall)
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

    // 3. Adjusting number of walls left for given player
    board->wallsLeft[player]++;
}


void UpdatePossibleMoves(Board_t* board, Player_t player)
{
    Tile_t* pT = GetPlayerTile(board, player);
    Tile_t* oT = GetPlayerTile(board, board->otherPlayer[player]);

    board->moves[player][MOVE_NORTH].isPossible = ((pT->north) && (pT->north != oT) ? true : false);
    board->moves[player][MOVE_SOUTH].isPossible = ((pT->south) && (pT->south != oT) ? true : false);
    board->moves[player][MOVE_EAST].isPossible = ((pT->east) && (pT->east != oT) ? true : false);
    board->moves[player][MOVE_WEST].isPossible = ((pT->west) && (pT->west != oT) ? true : false);

    board->moves[player][JUMP_NORTH].isPossible = ((pT->north) && (pT->north == oT) && (oT->north) ? true : false);
    board->moves[player][JUMP_SOUTH].isPossible = ((pT->south) && (pT->south == oT) && (oT->south) ? true : false);
    board->moves[player][JUMP_EAST].isPossible = ((pT->east) && (pT->east == oT) && (oT->east) ? true : false);
    board->moves[player][JUMP_WEST].isPossible = ((pT->west) && (pT->west == oT) && (oT->west) ? true : false);

    board->moves[player][JUMP_NORTH_EAST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->east)) ? true :          // jump N -> E
                                                        (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->north)) ? true : false)); // jump E -> N
    board->moves[player][JUMP_NORTH_WEST].isPossible = (((pT->north) && (pT->north == oT) && (!oT->north) && (oT->west)) ? true :          // jump N -> W
                                                        (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->north)) ? true : false)); // jump W -> N                          
    board->moves[player][JUMP_SOUTH_EAST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->east)) ? true :          // jump S -> E
                                                        (((pT->east) && (pT->east == oT) && (!oT->east) && (oT->south)) ? true : false)); // jump E -> S
    board->moves[player][JUMP_SOUTH_WEST].isPossible = (((pT->south) && (pT->south == oT) && (!oT->south) && (oT->west)) ? true :          // jump S -> W
                                                        (((pT->west) && (pT->west == oT) && (!oT->west) && (oT->south)) ? true : false)); // jump W -> S
}


// Check that move is possible BEFORE calling this!
void MakeMove(Board_t* board, Player_t player, MoveID_t moveID)
{
    board->playerPos[player].x += board->moves[player][moveID].xDiff;
    board->playerPos[player].y += board->moves[player][moveID].yDiff;
}

// Only call this in pair with MakeMove()
void UndoMove(Board_t* board, Player_t player, MoveID_t moveID)
{
    board->playerPos[player].x -= board->moves[player][moveID].xDiff;
    board->playerPos[player].y -= board->moves[player][moveID].yDiff;
}


static void DecreaseWallPermission(Wall_t* wall)
{
    if (wall)
    {
        wall->permission = (WallPermission_t)(wall->permission - 1);        
    }
}

static void IncreaseWallPermission(Wall_t* wall)
{
    if (wall)
    {
        wall->permission = (WallPermission_t)(wall->permission + 1);   
    }
}

Tile_t* GetPlayerTile(Board_t* board, Player_t player)
{
    return (&(board->tiles[board->playerPos[player].x][board->playerPos[player].y]));
}

bool HasPlayerWon(Board_t* board, Player_t player)
{
    return (GetPlayerTile(board, player)->isGoalFor == player ? true : false);
}

void DisableCornerWalls(Board_t* board)
{
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {
                if (x == 0 && y == 0)
                {
                    board->walls[o][x][y].isDisabled = true;
                }
                
                if (x == 0 && y == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = true;
                }

                if (y == 0 && x == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = true;
                }

                if (y == BOARD_SZ - 2 && x == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = true;
                }
            }
        }
    }
}

void EnableCornerWalls(Board_t*board)
{
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {
                if (x == 0 && y == 0)
                {
                    board->walls[o][x][y].isDisabled = false;
                }
                
                if (x == 0 && y == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = false;
                }

                if (y == 0 && x == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = false;
                }

                if (y == BOARD_SZ - 2 && x == BOARD_SZ - 2)
                {
                    board->walls[o][x][y].isDisabled = false;
                }
            }
        }
    }
}

void DisableAllWalls(Board_t* board)
{
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {
                board->walls[o][x][y].isDisabled = true;
            }
        }
    }
}

void EnableAllWalls(Board_t*board)
{
    for (int8_t o = H; o <= V; o++) // orientation V or H
    {
        for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
        {
            for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
            {               
                board->walls[o][x][y].isDisabled = false;               
            }
        }
    }
}


void DisableFirstAndLastColVertWalls(Board_t* board)
{
    for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
    {
        for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
        {
            if (y == 0 || y == (BOARD_SZ - 2)) // first or last col
            {
                board->walls[V][x][y].isDisabled = true;
            }
        }
    }
}

void EnableFirstAndLastColVertWalls(Board_t* board)
{
    for (int8_t x = 0; x < (BOARD_SZ - 1); x++)
    {
        for (int8_t y = 0; y < (BOARD_SZ - 1); y++)
        {
            if (y == 0 || y == (BOARD_SZ - 2)) // first or last col
            {
                board->walls[V][x][y].isDisabled = false;
            }
        }
    }
}

}

