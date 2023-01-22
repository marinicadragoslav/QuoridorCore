#ifndef Header_qplugin_marinica_board
#define Header_qplugin_marinica_board

#include <stdint.h>
#include <stdbool.h>

#define BOARD_SZ 9

typedef enum
{
   ME,
   OPPONENT
}Player_t;

typedef struct
{
   uint8_t x;
   uint8_t y;
}Position_t;

typedef struct Tile_t
{
   Position_t pos;
   struct Tile_t* north;
   struct Tile_t* south;
   struct Tile_t* east;
   struct Tile_t* west;
   bool isGoalForMe;
   bool isGoalForOpp;
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
   MOVE_ID_COUNT
}MoveID_t;

typedef struct Move_t
{
   MoveID_t id;
   bool isPossible;
   struct Move_t* next;
   struct Move_t* prev;   
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
   bool debug_isPossible;
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
   bool debug_isPossible;
}VWall_t;

typedef struct
{
   Position_t myPos;
   Position_t oppPos;
   uint8_t myWallsLeft;
   uint8_t oppWallsLeft;
   Tile_t tiles[BOARD_SZ][BOARD_SZ];
   HWall_t hWalls[BOARD_SZ - 1][BOARD_SZ - 1];
   VWall_t vWalls[BOARD_SZ - 1][BOARD_SZ - 1];
   Move_t moves[MOVE_ID_COUNT];
   HWall_t* hWallFirst;
   VWall_t* vWallFirst;
   Move_t* moveFirst;
}Board_t;

Board_t* GetBoard(void);
void InitBoard(void);
void UpdateMyPos(Position_t pos);
void UpdateOpponentPos(Position_t pos);
void UpdateMyWallsLeft(uint8_t n);
void UpdateOpponentWallsLeft(uint8_t n);
void PlaceHWallByMe(Position_t pos);
void PlaceHWallByOpponent(Position_t pos);
void PlaceVWallByMe(Position_t pos);
void PlaceVWallByOpponent(Position_t pos);
void UndoHWallByMe(Position_t pos);
void UndoHWallByOpponent(Position_t pos);
void UndoVWallByMe(Position_t pos);
void UndoVWallByOpponent(Position_t pos);

#endif // Header_qplugin_marinica_board
