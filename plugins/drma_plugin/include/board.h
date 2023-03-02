#ifndef Header_qplugin_drma_board
#define Header_qplugin_drma_board

#include <stdint.h>
#include <stdbool.h>
#include "drma_player.h"

namespace qplugin_drma 
{
   typedef enum
   {
      ME,
      OPPONENT,
      NOBODY
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


   typedef enum
   {
      MAKE_MOVE,
      PLACE_WALL,
      NULL_ACTION
   }Action_t;


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
      NULL_MOVE
   }MoveID_t;


   typedef struct Move_t
   {
      MoveID_t moveID;
      bool isPossible;
      int8_t xDiff;
      int8_t yDiff;
   }Move_t;


   typedef enum
   {
      WALL_PERMITTED = 3,
      WALL_FORBIDDEN_BY_1 = 2,
      WALL_FORBIDDEN_BY_2 = 1,
      WALL_FORBIDDEN_BY_3 = 0
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
      bool isEnabled;
   }Wall_t;


   typedef struct
   {
      Action_t action;
      MoveID_t moveID;
      Wall_t* wall;
   }Play_t;
   

   typedef struct
   {
      Player_t player;
      Action_t action;
      Move_t* move;
      Wall_t* wall;
   }NominalPlay_t;   


   typedef struct
   {
      Position_t playerPos[2];
      Player_t otherPlayer[2];
      uint8_t wallsLeft[2];
      Tile_t tiles[BOARD_SZ][BOARD_SZ];
      Wall_t walls[2][BOARD_SZ - 1][BOARD_SZ - 1];
      Move_t moves[2][MOVE_COUNT];
      bool isOnMinPath[BOARD_SZ][BOARD_SZ];
      NominalPlay_t plays[sizeof(walls)/sizeof(walls[0][0][0]) + sizeof(moves)/sizeof(moves[0][0])];
   }Board_t;


   typedef enum
   {
      ALL_WALLS,
      CORNER_WALLS,
      VERT_WALLS_FIRST_LAST_COL
   }WallsSubset_t;


    // Create a new board and initialize the structure for it
   Board_t* NewDefaultBoard(void);


   Wall_t* GetWallByPosAndOrientation(Board_t* board, Position_t wallPos, Orientation_t wallOr);
   Tile_t* GetPlayerTile(Board_t* board, Player_t player);
   void UpdatePos(Board_t* board, Player_t player, Position_t pos);
   void UpdateWallsLeft(Board_t* board, Player_t player, uint8_t wallsLeft);
   void PlaceWall(Board_t* board, Player_t player, Wall_t* wall);
   void UndoWall(Board_t* board, Player_t player, Wall_t* wall);
   void UpdatePossibleMoves(Board_t* board, Player_t player);
   bool HasPlayerWon(Board_t* board, Player_t player);
   void MakeMove(Board_t* board, Player_t player, MoveID_t moveID);
   void UndoMove(Board_t* board, Player_t player, MoveID_t moveID);
   void EnableWallsSubset(Board_t* board, WallsSubset_t subset);
   void DisableWallsSubset(Board_t* board, WallsSubset_t subset);

} // end namespace
#endif // Header_qplugin_drma_board
