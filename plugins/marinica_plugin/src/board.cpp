
#include <stddef.h>
#include <string.h>
#include "board.h"

#define L (BOARD_SZ - 1)
#define WALL_POSSIBLE 1U

static void ForbidHorizWall(HWall_t* wall);
static void ForbidVertWall(VWall_t* wall);
static void AllowHorizWall(HWall_t* wall);
static void AllowVertWall(VWall_t* wall);
static RelativePlayerPos_t GetPlayersRelativePos(Player_t player);

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
            board.tiles[x][y].isGoalFor = ((x == 0) ? ME : ((x == L) ? OPPONENT : NONE));           
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
    board.wallsLeft[ME] = 10;
    board.wallsLeft[OPPONENT] = 10;

    // set initial player positions
    board.playerPos[ME] = {L, BOARD_SZ / 2};
    board.playerPos[OPPONENT] = {0, BOARD_SZ / 2};

    // set other player of each player :)
    board.otherPlayer[ME] = OPPONENT;
    board.otherPlayer[OPPONENT] = ME;
}

void UpdatePos(Player_t player, Position_t pos)
{
    board.playerPos[player] = pos;
}

void UpdateWallsLeft(Player_t player, uint8_t wallsLeft)
{
    board.wallsLeft[player] = wallsLeft;
}

void PlaceHorizWall(Player_t player, Position_t pos)
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

    // adjust number of walls left
    board.wallsLeft[player]--;
}

void PlaceVertWall(Player_t player, Position_t pos)
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

    // adjust number of walls left
    board.wallsLeft[player]--;
}

void UndoHorizWall(Player_t player, Position_t pos)
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

    // adjust number of walls left
    board.wallsLeft[player]++;
}

void UndoVertWall(Player_t player, Position_t pos)
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

    // adjust number of walls left
    board.wallsLeft[player]++;
}

void UpdatePossibleMoves(Player_t player)  // optimize this FFS! maybe
{
    // first, set all moves to NOT POSSIBLE
    memset(board.moves[player], 0, sizeof(board.moves[player]));

    // get player's tile
    Tile_t* playerTile = &(board.tiles[board.playerPos[player].x][board.playerPos[player].y]);
    Player_t other = board.otherPlayer[player];
    Tile_t* otherPlayerTile = &(board.tiles[board.playerPos[other].x][board.playerPos[other].y]);

    switch (GetPlayersRelativePos(player))
    {
        case NOT_SIDE_BY_SIDE: // most often                       
            // no jumps possible, so only worry about one-step-moves:               
            board.moves[player][MOVE_NORTH].isPossible = (playerTile->north ? true : false);
            board.moves[player][MOVE_SOUTH].isPossible = (playerTile->south ? true : false);
            board.moves[player][MOVE_EAST].isPossible = (playerTile->east ? true : false);
            board.moves[player][MOVE_WEST].isPossible = (playerTile->west ? true : false);
        break;

        case PLAYER_BELOW:
            // moving one step north not possible, check for jumping:
            if (playerTile->north) // no wall above player
            {
                if (otherPlayerTile->north) // no wall or border above other player
                {
                    board.moves[player][JUMP_NORTH].isPossible = true; // allow jumping straight over
                }
                else // wall or border above other player
                {
                    if (otherPlayerTile->west) // no wall or border to other player's left
                    {
                        board.moves[player][JUMP_NORTH_WEST].isPossible = true;
                    }
                    if (otherPlayerTile->east) // no wall or border to other player's right
                    {
                        board.moves[player][JUMP_NORTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.moves[player][MOVE_SOUTH].isPossible = (playerTile->south ? true : false);
            board.moves[player][MOVE_EAST].isPossible = (playerTile->east ? true : false);
            board.moves[player][MOVE_WEST].isPossible = (playerTile->west ? true : false);
        break;

        case PLAYER_ABOVE:
            // moving one step south not possible, check for jumping:
            if (playerTile->south) // no wall below player
            {
                if (otherPlayerTile->south) // no wall or border below other player
                {
                    board.moves[player][JUMP_SOUTH].isPossible = true; // allow jumping straight over
                }
                else // wall or border below other player
                {
                    if (otherPlayerTile->west) // no wall or border to other player's left
                    {
                        board.moves[player][JUMP_SOUTH_WEST].isPossible = true;
                    }
                    if (otherPlayerTile->east) // no wall or border to other player's right
                    {
                        board.moves[player][JUMP_SOUTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.moves[player][MOVE_NORTH].isPossible = (playerTile->north ? true : false);
            board.moves[player][MOVE_EAST].isPossible = (playerTile->east ? true : false);
            board.moves[player][MOVE_WEST].isPossible = (playerTile->west ? true : false);
        break;

        case PLAYER_ON_THE_RIGHT:
            // moving one step west not possible, check for jumping:
            if (playerTile->west) // no wall to player's left
            {
                if (otherPlayerTile->west) // no wall or border to other player's left
                {
                    board.moves[player][JUMP_WEST].isPossible = true; // allow jumping straight over
                }
                else // wall or border to other player's left
                {
                    if (otherPlayerTile->north) // no wall or border above other player
                    {
                        board.moves[player][JUMP_NORTH_WEST].isPossible = true;
                    }
                    if (otherPlayerTile->south) // no wall or border below other player
                    {
                        board.moves[player][JUMP_SOUTH_WEST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.moves[player][MOVE_NORTH].isPossible = (playerTile->north ? true : false);
            board.moves[player][MOVE_SOUTH].isPossible = (playerTile->south ? true : false);
            board.moves[player][MOVE_EAST].isPossible = (playerTile->east ? true : false);
        break;

        case PLAYER_ON_THE_LEFT:
            // moving one step east not possible, check for jumping:
            if (playerTile->east) // no wall to player's right
            {
                if (otherPlayerTile->east) // no wall or border to other player's right
                {
                    board.moves[player][JUMP_EAST].isPossible = true; // allow jumping straight over
                }
                else // wall or border to other player's right
                {
                    if (otherPlayerTile->north) // no wall or border above other player
                    {
                        board.moves[player][JUMP_NORTH_EAST].isPossible = true;
                    }
                    if (otherPlayerTile->south) // no wall or border below other player
                    {
                        board.moves[player][JUMP_SOUTH_EAST].isPossible = true;
                    }
                }
            }
            // check for possible one-step-moves in the other directions
            board.moves[player][MOVE_NORTH].isPossible = (playerTile->north ? true : false);
            board.moves[player][MOVE_SOUTH].isPossible = (playerTile->south ? true : false);
            board.moves[player][MOVE_WEST].isPossible = (playerTile->west ? true : false);
        break;

        default: // this can't happen
        break;
    }
}

static RelativePlayerPos_t GetPlayersRelativePos(Player_t player)
{
    Player_t otherPlayer = board.otherPlayer[player];

    if (board.playerPos[player].x == board.playerPos[otherPlayer].x)
    {
        if (board.playerPos[player].y == board.playerPos[otherPlayer].y - 1)
        {
            return PLAYER_ON_THE_LEFT;
        }
        if (board.playerPos[player].y == board.playerPos[otherPlayer].y + 1)
        {
            return PLAYER_ON_THE_RIGHT;
        }
    }

    if (board.playerPos[player].y == board.playerPos[otherPlayer].y)
    {
        if (board.playerPos[player].x == board.playerPos[otherPlayer].x - 1)
        {
            return PLAYER_ABOVE;
        }
        if (board.playerPos[player].x == board.playerPos[otherPlayer].x + 1)
        {
            return PLAYER_BELOW;
        }
    }

    return NOT_SIDE_BY_SIDE;
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