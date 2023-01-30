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

typedef struct HorizWallsListItem_t
{
   HWall_t* wall;
   HorizWallsListItem_t* next;
   HorizWallsListItem_t* prev;
   bool debug_isRemoved;
   bool debug_isPrintedAsRemoved;
   bool debug_isAdded;
   bool debug_isPrintedAsAdded;
}HorizWallsListItem_t;

typedef struct VertWallsListItem_t
{
   VWall_t* wall;
   struct VertWallsListItem_t* next;
   struct VertWallsListItem_t* prev;
   bool debug_isRemoved;
   bool debug_isPrintedAsRemoved;
   bool debug_isAdded;
   bool debug_isPrintedAsAdded;
}VertWallsListItem_t;

typedef struct
{
   Position_t myPos;
   Position_t oppPos;
   uint8_t myWallsLeft;
   uint8_t oppWallsLeft;

   Tile_t tiles[BOARD_SZ][BOARD_SZ];
   HWall_t hWalls[BOARD_SZ - 1][BOARD_SZ - 1];
   VWall_t vWalls[BOARD_SZ - 1][BOARD_SZ - 1];

   HorizWallsListItem_t possibleHorizWallsList[(BOARD_SZ - 1) * (BOARD_SZ - 1)];
   HorizWallsListItem_t* headPHWL; // head of Possible Horiz Walls List

   VertWallsListItem_t possibleVertWallsList[(BOARD_SZ - 1) * (BOARD_SZ - 1)];
   VertWallsListItem_t* headPVWL; // head of Possible Vert Walls List

   Move_t myMoves[MOVE_COUNT];
   Move_t oppMoves[MOVE_COUNT];

   bool debug_isOnMyMinPath[BOARD_SZ][BOARD_SZ];
}Board_t;

typedef enum RelativePlayerPos_t
{
   NOT_FACE_TO_FACE,
   OPPONENT_ABOVE_ME,
   OPPONENT_BELOW_ME,
   OPPONENT_TO_MY_LEFT,
   OPPONENT_TO_MY_RIGHT
}RelativePlayerPos_t;

Board_t* GetBoard(void);
void InitBoard(void);
void UpdateMyPos(Position_t pos);
void UpdateOpponentsPos(Position_t pos);
void UpdateMyWallsLeft(uint8_t n);
void UpdateOpponentWallsLeft(uint8_t n);
void PlaceHorizWallByMe(Position_t pos);
void PlaceHorizWallByOpponent(Position_t pos);
void PlaceVertWallByMe(Position_t pos);
void PlaceVertWallByOpponent(Position_t pos);
void UndoHorizWallByMe(Position_t pos);
void UndoHorizWallByOpponent(Position_t pos);
void UndoVertWallByMe(Position_t pos);
void UndoVertWallByOpponent(Position_t pos);
void UpdateMyPossibleMoves(void);
//void UpdateOpponentsPossibleMoves(void);
bool IsMyPos(Position_t pos);
bool IsOpponentsPos(Position_t pos);

#endif // Header_qplugin_marinica_board
