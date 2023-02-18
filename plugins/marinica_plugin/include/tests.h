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


#endif // Header_qplugin_marinica_debug
