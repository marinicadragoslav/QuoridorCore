#ifndef Header_qplugin_marinica_board
#define Header_qplugin_marinica_board

#include <stdint.h>
#include <stdbool.h>

#define BOARD_SZ 9
#define USE_POSSIBLE_WALLS_LIST 1


typedef enum
{
   ME,
   OPPONENT,
   NONE
}Player_t;


typedef enum
{
   H,
   V
}Orientation_t;


typedef struct
{
   int8_t x;
   int8_t y;
}Position_t;


typedef struct Tile_t
{
   Position_t pos;
   struct Tile_t* west;
   struct Tile_t* south;
   struct Tile_t* east;
   struct Tile_t* north;
   Player_t isGoalFor;
}Tile_t;


typedef enum
{
   MOVE_NORTH,
   MOVE_SOUTH,
   MOVE_EAST,
   MOVE_WEST,
   JUMP_NORTH,
   JUMP_SOUTH,
   JUMP_EAST,
   JUMP_WEST,
   JUMP_NORTH_EAST,
   JUMP_NORTH_WEST,
   JUMP_SOUTH_EAST,
   JUMP_SOUTH_WEST,
   
   MOVE_COUNT,
   MOVE_FIRST = MOVE_NORTH,
   MOVE_LAST = JUMP_SOUTH_WEST,
}MoveID_t;


typedef struct Move_t
{
   bool isPossible; 
}Move_t;


typedef enum
{
    WALL_PERMITTED = 3,
    WALL_FORBIDDEN_BY_1_OTHER_WALL = 2,
    WALL_FORBIDDEN_BY_2_OTHER_WALLS = 1,
    WALL_FORBIDDEN_BY_3_OTHER_WALLS = 0
}WallPermission_t;


typedef struct Wall_t
{
   Position_t pos;
   Orientation_t orientation;
   Tile_t* northwest;
   Tile_t* northeast;
   Tile_t* southwest;
   Tile_t* southeast;
   struct Wall_t* forbidsPrev;
   struct Wall_t* forbidsNext;
   struct Wall_t* forbidsCompl;
   WallPermission_t permission;
   struct Wall_t* possibleNext;
   struct Wall_t* possiblePrev;
}Wall_t;


typedef struct
{
   Position_t playerPos[2];
   Player_t otherPlayer[2];
   uint8_t wallsLeft[2];
   Tile_t tiles[BOARD_SZ][BOARD_SZ];
   Wall_t walls[2][BOARD_SZ - 1][BOARD_SZ - 1];
   Move_t moves[2][MOVE_COUNT];
   bool debug_isOnMinPath[BOARD_SZ][BOARD_SZ];
   Wall_t* firstPossibleWall;
}Board_t;




void InitBoard(void);
Board_t* GetBoard(void);
Wall_t* GetWall(Position_t wallPos, Orientation_t wallOr);
Tile_t* GetPlayerTile(Player_t player);
void UpdatePos(Player_t player, Position_t pos);
void UpdateWallsLeft(Player_t player, uint8_t wallsLeft);
void PlaceWall(Player_t player, Wall_t* wall);
void UndoWall(Player_t player, Wall_t* wall);
void UpdatePossibleMoves(Player_t player);


#endif // Header_qplugin_marinica_board
