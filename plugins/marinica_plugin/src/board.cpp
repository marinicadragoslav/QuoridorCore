
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
static void RemoveFromPossibleHorizWallsList(HWall_t* wall);
static void RemoveFromPossibleVertWallsList(VWall_t* wall);
static void AddToPossibleHorizWallsList(HWall_t* wall);
static void AddToPossibleVertWallsList(VWall_t* wall);
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

            // add to the linked list of possible vertical walls and set links
            int8_t i = x * L + y;
            board.possibleHorizWallsList[i].wall = &(board.hWalls[x][y]);
            board.possibleHorizWallsList[i].prev = ((x == 0 && y == 0) ?         NULL : &(board.possibleHorizWallsList[i - 1]));
            board.possibleHorizWallsList[i].next = ((x == L - 1 && y == L - 1) ? NULL : &(board.possibleHorizWallsList[i + 1]));

            // debug
            board.possibleHorizWallsList[i].debug_isRemoved = false;
            board.possibleHorizWallsList[i].debug_isPrintedAsRemoved = false;
            board.possibleHorizWallsList[i].debug_isAdded = false;
            board.possibleHorizWallsList[i].debug_isPrintedAsAdded = false;

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

            // add to the linked list of possible vertical walls and set links
            int8_t i = x * L + y;
            board.possibleVertWallsList[i].wall = &(board.vWalls[x][y]);
            board.possibleVertWallsList[i].prev = ((x == 0 && y == 0) ?         NULL : &(board.possibleVertWallsList[i - 1]));
            board.possibleVertWallsList[i].next = ((x == L - 1 && y == L - 1) ? NULL : &(board.possibleVertWallsList[i + 1]));

            // debug
            board.possibleHorizWallsList[i].debug_isRemoved = false;
            board.possibleHorizWallsList[i].debug_isPrintedAsRemoved = false;
            board.possibleHorizWallsList[i].debug_isAdded = false;
            board.possibleHorizWallsList[i].debug_isPrintedAsAdded = false;
        }
    }

    // init linked list heads
    board.headPHWL = &(board.possibleHorizWallsList[0]);
    board.headPVWL = &(board.possibleVertWallsList[0]);

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

/*
void UpdateOpponentsPossibleMoves(void)  // optimize this FFS! maybe
{
    if (board.PMsNeedUpdate)
    {
        // first, set all moves to NOT POSSIBLE
        memset(board.oppMoves, 0, sizeof(board.oppMoves));

        // get my tile & opponent's tile
        Tile_t* myTile = &(board.tiles[board.myPos.x][board.myPos.y]);   
        Tile_t* oppTile = &(board.tiles[board.oppPos.x][board.oppPos.y]);      

        switch (GetPlayersRelativePos())
        {
            case NOT_FACE_TO_FACE: // most often                
                // no jumps possible, so only worry about one-step-moves:               
                board.oppMoves[MOVE_NORTH].isPossible = (oppTile->north ? true : false);
                board.oppMoves[MOVE_SOUTH].isPossible = (oppTile->south ? true : false);
                board.oppMoves[MOVE_EAST].isPossible = (oppTile->east ? true : false);
                board.oppMoves[MOVE_WEST].isPossible = (oppTile->west ? true : false);
            break;

            case OPPONENT_ABOVE_ME:


        }
        if (!ArePlayersFaceToFace())
        {
            
        }
        else // players are face to face so this is more complicated - luckily it won't happen too often
        {
            // North
            if (oppTile->north) // no wall or border above him
            {
                if ((IsMyPos(oppTile->north->pos))) // I am above him
                {
                    if (myTile->north) // no wall or border above me
                    {
                        board.oppMoves[JUMP_NORTH].isPossible = true; // allow jumping straight over
                    }
                    else // wall or border above me
                    {
                        if (myTile->west) // no wall or border to my left
                        {
                            board.oppMoves[JUMP_NORTH_WEST].isPossible = true;
                        }
                        if (myTile->east) // no wall or border to my right
                        {
                            board.oppMoves[JUMP_NORTH_EAST].isPossible = true;
                        }
                    }
                }
                else // there is a free tile above him
                {
                    board.oppMoves[MOVE_NORTH].isPossible = true;
                }
            }

            			// South
            if (oppTile->south) // no wall or border below him
            {
                if ((IsMyPos(oppTile->south->pos))) // I am below him
                {
                    if (myTile->south) // no wall or border below me
                    {
                        board.oppMoves[JUMP_SOUTH].isPossible = true; // allow jumping straight over
                    }
                    else // wall or border below me
                    {
                        if (myTile->west) // no wall or border to my left
                        {
                            board.oppMoves[JUMP_SOUTH_WEST].isPossible = true;
                        }
                        if (myTile->east) // no wall or border to my right
                        {
                            board.oppMoves[JUMP_SOUTH_EAST].isPossible = true;
                        }
                    }
                }
                else // there is a free tile below him
                {
                    board.oppMoves[MOVE_SOUTH].isPossible = true;
                }
            }

            // West
            if (oppTile->west) // no wall or border to his left
            {
                if ((IsMyPos(oppTile->west->pos))) // I am on his left
                {
                    if (myTile->west) // no wall or border to my left
                    {
                        board.oppMoves[JUMP_WEST].isPossible = true; // allow jumping straight over
                    }
                    else // wall or border to my left
                    {
                        if (myTile->north) // no wall or border above me
                        {
                            board.oppMoves[JUMP_NORTH_WEST].isPossible = true;
                        }
                        if (myTile->south) // no wall or border below me
                        {
                            board.oppMoves[JUMP_SOUTH_WEST].isPossible = true;
                        }
                    }
                }
                else // there is a free tile to his left
                {
                    board.oppMoves[MOVE_WEST].isPossible = true;
                }
            }

            // East
            if (oppTile->east) // no wall or border to his right
            {
                if ((IsMyPos(oppTile->east->pos))) // I am on his right
                {
                    if (myTile->east) // no wall or border to my right
                    {
                        board.oppMoves[JUMP_EAST].isPossible = true; // allow jumping straight over
                    }
                    else // wall or border to my right
                    {
                        if (myTile->north) // no wall or border above me
                        {
                            board.oppMoves[JUMP_NORTH_EAST].isPossible = true;
                        }
                        if (myTile->south) // no wall or border below me
                        {
                            board.oppMoves[JUMP_SOUTH_EAST].isPossible = true;
                        }
                    }
                }
                else // there is a free tile to his right
                {
                    board.oppMoves[MOVE_EAST].isPossible = true;
                }
            }
        }
    }
}
*/

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
    HorizWallsListItem_t* removedItem; // debug 

    if ((board.headPHWL)->wall == wall)
    {
        // head to be removed so just update head
        removedItem = board.headPHWL; // debug
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
        removedItem = item; // debug
    }

    // debug - Destroy links and mark as removed
    removedItem->prev = NULL;
    removedItem->next = NULL;
    removedItem->debug_isRemoved = true;
}

static void RemoveFromPossibleVertWallsList(VWall_t* wall)
{
    VertWallsListItem_t* removedItem; // debug 

    if ((board.headPVWL)->wall == wall)
    {
        // head to be removed so just update head
        removedItem = board.headPVWL; // debug
        board.headPVWL = (board.headPVWL)->next;
        (board.headPVWL)->prev = NULL;
    }
    else
    {
        // find item
        int8_t index = wall->pos.x * L + wall->pos.y;
        VertWallsListItem_t* item = &(board.possibleVertWallsList[index]);
        removedItem = item; // debug

        // bypass item
        item->prev->next = item->next;
        if (item->next)
        {
            item->next->prev = item->prev;
        }
    }

    // debug - Destroy links and mark as removed
    removedItem->prev = NULL;
    removedItem->next = NULL;
    removedItem->debug_isRemoved = true;
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

    // debug
    board.headPHWL->debug_isAdded = true;
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

    // debug
    board.headPVWL->debug_isAdded = true;
}

