#ifndef Header_qplugin_marinica_board
#define Header_qplugin_marinica_board

#include <stdint.h>
#include <stdbool.h>

#define BOARD_SZ 9

typedef enum
{
   ME,
   OPPONENT,
   NONE
}Player_t;

typedef struct
{
   int8_t x;
   int8_t y;
}Position_t;

typedef struct Tile_t
{
   Position_t pos;
   struct Tile_t* north;
   struct Tile_t* south;
   struct Tile_t* east;
   struct Tile_t* west;
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

typedef struct HWall_t
{
   Position_t pos;
   uint8_t possibleFlag;
   struct HWall_t* east;
   struct HWall_t* west;
   Tile_t* northwest;
   Tile_t* northeast;
   Tile_t* southwest;
   Tile_t* southeast;
}HWall_t;

typedef struct VWall_t
{
   Position_t pos;
   uint8_t possibleFlag;
   struct VWall_t* north;
   struct VWall_t* south;
   Tile_t* northwest;
   Tile_t* northeast;
   Tile_t* southwest;
   Tile_t* southeast;
}VWall_t;

typedef struct
{
   Position_t playerPos[2]; // playerPos[ME], playerPos[OPPONENT]
   Player_t otherPlayer[2]; // otherPlayer[ME] == OPPONENT, otherPlayer[OPPONENT] == ME
   uint8_t wallsLeft[2]; // wallsLeft[ME], wallsLeft[OPPONENT]
   Tile_t tiles[BOARD_SZ][BOARD_SZ];
   HWall_t hWalls[BOARD_SZ - 1][BOARD_SZ - 1]; // Horizontal walls
   VWall_t vWalls[BOARD_SZ - 1][BOARD_SZ - 1]; // Vertical walls
   Move_t moves[2][MOVE_COUNT]; // e.g. moves[ME][JUMP_NORTH], moves[OPPONENT][MOVE_WEST]
   bool debug_isOnMinPath[BOARD_SZ][BOARD_SZ];
}Board_t;

typedef enum RelativePlayerPos_t
{
   NOT_SIDE_BY_SIDE,
   PLAYER_ABOVE,
   PLAYER_BELOW,
   PLAYER_ON_THE_LEFT,
   PLAYER_ON_THE_RIGHT,
}RelativePlayerPos_t;

Board_t* GetBoard(void);
void InitBoard(void);
void UpdatePos(Player_t player, Position_t pos);
void UpdateWallsLeft(Player_t player, uint8_t wallsLeft);
void PlaceHorizWall(Player_t player, Position_t pos);
void PlaceVertWall(Player_t player, Position_t pos);
void UndoHorizWall(Player_t player, Position_t pos);
void UndoVertWall(Player_t player, Position_t pos);
void UpdatePossibleMoves(Player_t player);

#endif // Header_qplugin_marinica_board
