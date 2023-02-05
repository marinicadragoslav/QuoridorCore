#ifndef Header_qplugin_marinica_tests
#define Header_qplugin_marinica_tests

#include "board.h"

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

void test_1_CheckInitialBoardStructure(Board_t* board);
void test_2_PlaceOneHorizWallThatIsNotOnTheBorder(Board_t* board);
void test_3_UndoLastWall(Board_t* board);
void test_4_PlaceTwoConsecutiveHorizWalls(Board_t* board);
void test_5_UndoLastTwoWallsOneByOne(Board_t* board);
void test_6_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(Board_t* board);
void test_7_Place2VertWallsAndOneHorizWallAndThenUndoAll(Board_t* board);
void test_8_PlaceAndUndoGroupsOf3Walls(Board_t* board);
void test_9_MinPathAndPossibleMoves(void);
void test_10_MinPathAndPossibleMoves(void);
void test_11_MinPathAndPossibleMoves(void);
void test_12_MinPathAndPossibleMoves(void);
void test_13_MinPathAndPossibleMoves(void);


#endif // Header_qplugin_marinica_debug
