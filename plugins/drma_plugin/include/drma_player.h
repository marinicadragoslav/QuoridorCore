#ifndef H_QPLUGIN_DRMA_PLAYER
#define H_QPLUGIN_DRMA_PLAYER

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <chrono>
#include "Player.h"
#include "QcoreUtil.h"

#define RUN_TESTS                      (false)
#define PRINT_DEBUG_INFO               (true) // only has effect when RUN_TESTS is false
#define LOGGED_LINE_MAX_LEN            (120)

#define BOARD_SZ                       (9)

#define COUNT(arr)                     (sizeof(arr) / sizeof(arr[0]))
#define COUNT_2D(arr)                  (sizeof(arr) / sizeof(arr[0][0]))
#define COUNT_3D(arr)                  (sizeof(arr) / sizeof(arr[0][0][0]))

#define MINIMAX_DEPTH                  (3)
#define MINIMAX_TIMEOUT_MS             (4900)
#define POS_INFINITY                   (+0xFFFFFF)     // some large positive value
#define NEG_INFINITY                   (-0xFFFFFF)     // some large negative value
#define CAP_POS_SCORE                  (+0xFFFFF0)     // valid minimax score < CAP_POS_SCORE < POS_INFINITY
#define CAP_NEG_SCORE                  (-0xFFFFF0)     // valid minimax score > CAP_NEG_SCORE > NEG_INFINITY
#define ERROR_NO_PATH                  (-0xFFFFFFF)    // must be outside [NEG_INFINITY, POS_INFINITY] range
#define IS_VALID(score)                (CAP_NEG_SCORE <= score && score <= CAP_POS_SCORE)

#define INFINITE_LEN                   (0xFFU)
#define QUEUE_MAX_SIZE                 (1000)

#define DECREASE_WALL_PERMISSION(wall)  (wall && (wall->permission = (WallPermission_t)(wall->permission - 1)))
#define INCREASE_WALL_PERMISSION(wall)  (wall && (wall->permission = (WallPermission_t)(wall->permission + 1)))

#if (RUN_TESTS)
#define PRINT_DEBUG_INFO               (true)
#define DEFAULT_NULL_TILE_LINKS        { 0, 0, N }, \
                                       { 0, 0, W }, \
                                       { 0, 1, N }, \
                                       { 0, 2, N }, \
                                       { 0, 3, N }, \
                                       { 0, 4, N }, \
                                       { 0, 5, N }, \
                                       { 0, 6, N }, \
                                       { 0, 7, N }, \
                                       { 0, 8, N }, \
                                       { 0, 8, E }, \
                                       { 1, 0, W }, \
                                       { 1, 8, E }, \
                                       { 2, 0, W }, \
                                       { 2, 8, E }, \
                                       { 3, 0, W }, \
                                       { 3, 8, E }, \
                                       { 4, 0, W }, \
                                       { 4, 8, E }, \
                                       { 5, 0, W }, \
                                       { 5, 8, E }, \
                                       { 6, 0, W }, \
                                       { 6, 8, E }, \
                                       { 7, 0, W }, \
                                       { 7, 8, E }, \
                                       { 8, 0, W }, \
                                       { 8, 0, S }, \
                                       { 8, 1, S }, \
                                       { 8, 2, S }, \
                                       { 8, 3, S }, \
                                       { 8, 4, S }, \
                                       { 8, 5, S }, \
                                       { 8, 6, S }, \
                                       { 8, 7, S }, \
                                       { 8, 8, S }, \
                                       { 8, 8, E }
#endif

namespace qplugin
{
   class drmaPlayer : public qcore::Player
   {
   public:

      /** Construction */
      drmaPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game);

      /** Defines player's behavior. In this particular case, it's a really dummy one */
      void doNextMove() override;

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
         NominalPlay_t plays[COUNT_3D(walls) + COUNT_2D(moves)];
      }Board_t;


      typedef enum
      {
         ALL_WALLS,
         CORNER_WALLS,
         VERT_WALLS_FIRST_LAST_COL
      }WallsSubset_t;


      typedef enum
      {
         N, S, E, W
      }Dir_t;


      typedef struct 
      {
         Orientation_t ori;
         int8_t x;
         int8_t y;
      }TestWall_t;


      typedef struct
      {
         int8_t x;
         int8_t y;
         Dir_t dir;
      }TestTileLink_t;


      typedef struct 
      {
         Orientation_t ori;
         int8_t x;
         int8_t y;
         WallPermission_t permission;
      }TestWallPermission_t;


      typedef struct
      {
         Tile_t* tile;
         Tile_t* prevTile;
         uint8_t pathLen;
      }Subpath_t;

      // Transformations
      qcore::Position CoreAbsToRelWallPos(qcore::Position absPos, qcore::Orientation orientation);
      Position_t CoreToPluginWallPos(qcore::Position coreWallPos, qcore::Orientation orientation);
      qcore::Position PluginToCoreWallPos(Position_t pluginWallPos, Orientation_t orientation);
      Orientation_t CoreToPluginWallOrientation(qcore::Orientation orientation);
      qcore::Orientation PluginToCoreWallOrientation(Orientation_t orientation);

      // Board
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

      // Min path
      uint8_t FindMinPathLen(Board_t* board, Player_t player, bool* found);
      void QueueInit(void);
      bool IsQueueEmpty(void);
      void QueuePush(Subpath_t item);
      Subpath_t* QueuePop(void);
      void FoundSubpathsInit(void);
      bool IsMinPathFoundForTile(Tile_t* tile);

      // Minimax
      int Minimax(Board_t* board, Player_t player, uint8_t level, int alpha, int beta, 
                    std::chrono::time_point<std::chrono::steady_clock> tStart, bool canTimeOut, bool *hasTimedOut);

      Play_t GetBestPlayForLevel(uint8_t level);
      int StaticEval(Board_t* board);

      // Logging and debugging
      #if (PRINT_DEBUG_INFO)
      void ClearBuff(void);
      const char* debug_ConvertMoveIDToString(MoveID_t moveID);
      char* debug_PrintMyPossibleMoves(Board_t* board);
      char* debug_PrintOppPossibleMoves(Board_t* board);
      void debug_PrintPlay(Play_t play);
      void debug_PrintBoard(Board_t* board);
      #endif

      // Testing
      #if (PRINT_DEBUG_INFO && RUN_TESTS)
      void debug_PrintTestFailed(void);
      void debug_PrintTestPassed(void);
      void debug_PrintTestErrorMsg(const char* errMsg);
      void debug_PrintTestMessage(const char* msg);
      void debug_PrintTestMinPaths(int minPathMe, int minPathOpp);

      void PlaceWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount);
      void UndoWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount);
      void CheckBoardStructure(Board_t* board, TestTileLink_t* tileLinksToTest, int8_t tileLinksCount,
                     TestWallPermission_t* permissionsToCheck, int8_t permissionsToCheckCount);
      void StringToMap(const char* stringInput, char* mapOutput, int8_t* myPosX, int8_t* myPosY, int8_t* oppPosX, int8_t* oppPosY);
      int TestGetMinPathToTargetTile(char* map, int playerX, int playerY, int targetX, int targetY);
      int TestGetMinPathForMe(char* map, int myX, int myY);
      int TestGetMinPathForOpp(char* map, int oppX, int oppY);
      void MapToBoard(char* map, Board_t* board, int8_t myMapPosX, int8_t myMapPosY, int8_t oppMapPosX, int8_t oppMapPosY);
      bool IsMinPathAndPossibleMovesTestPassed(const char* stringInput, const char* possibleMovesMeInput, const char* possibleMovesOppInput);
      void DuplicateBoard(Board_t* origBoard, Board_t* copyBoard);
      bool IsRecursiveRunOkForTestPossibleMoves(Board_t* board, Board_t* referenceBoard, uint8_t level);
      bool AreBoardsEqual(Board_t* b1, Board_t* b2);

      void test_1_CheckInitialBoardStructure(Board_t* board);
      void test_2_PlaceThenUndoOneHorizWallThatIsNotOnTheBorder(Board_t* board);
      void test_3_PlaceThenUndoOneVertWallThatIsNotOnTheBorder(Board_t* board);
      void test_4_PlaceThenUndoOneHorizWallThatIsOnTheBorder(Board_t* board);
      void test_5_PlaceThenUndoOneVertWallThatIsOnTheBorder(Board_t* board);
      void test_6_PlaceTwoConsecutiveHorizWallsAndUndoThem(Board_t* board);
      void test_7_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(Board_t* board);
      void test_8_Place2VertWallsAndOneHorizWallAndThenUndoAll(Board_t* board);
      void test_9_PlaceAndUndoGroupsOf3Walls(Board_t* board);
      void test_10_MinPathAndPossibleMoves(void);
      void test_11_MinPathAndPossibleMoves(void);
      void test_12_MinPathAndPossibleMoves(void);
      void test_13_MinPathAndPossibleMoves(void);
      void test_14_MinPathAndPossibleMoves(void);
      void test_15_MinPathAndPossibleMoves(void);
      void test_16_MinPathAndPossibleMoves(void);
      void test_17_MinPathAndPossibleMoves(void);
      void test_18_MinPathAndPossibleMoves(void);
      void test_19_MinPathAndPossibleMoves(void);
      void test_20_MinPathAndPossibleMoves(void);
      void test_21_MinPathAndPossibleMoves(void);
      void test_22_MinPathAndPossibleMoves(void);
      void test_23_TestPossibleMovesRecursiveCorrectnessDefaultPlayerPos(Board_t* board, uint8_t level);
      void test_24_TestPossibleMovesRecursiveCorrectnessDifferentPlayerPos(Board_t* board, uint8_t level);
      void RunAllTests(Board_t* board);
      #endif


   private:
      char buff[LOGGED_LINE_MAX_LEN] = {0};
      uint16_t turn = 0;
      Board_t* board;
      Subpath_t queue[QUEUE_MAX_SIZE];
      uint16_t queueNext, queueFirst;
      Subpath_t foundSubpaths[BOARD_SZ][BOARD_SZ];
      uint16_t goalTilesReached;
      Subpath_t* destination;
      Play_t bestPlays[MINIMAX_DEPTH + 1];
      bool areAllWallsDisabled = false;
      bool areCornerWallsDisabled = false;
      bool areFirstAndLastColVertWallsDisabled = false;

   };
   
} // end namespace
#endif // H_QPLUGIN_DRMA_PLAYER
