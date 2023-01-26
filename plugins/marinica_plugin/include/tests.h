#ifndef Header_qplugin_marinica_tests
#define Header_qplugin_marinica_tests

#include "board.h"

typedef enum
{
    H, V
}WallOr_t;

typedef enum
{
    N, S, E, W
}Dir_t;

typedef enum
{
    POSSIBLE = 1,
    FORBIDDEN_1X = 2,
    FORBIDDEN_2X = 4,
    FORBIDDEN_3x = 8
}PossibilityFlag_t;

typedef struct 
{
    WallOr_t wallOr;
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
    WallOr_t wallOr;
    int8_t x;
    int8_t y;
    PossibilityFlag_t possibilityFlag;
}TestPossibilityFlag_t;




void test_1_CheckInitialBoardStructure(Board_t* board);
void test_2_PlaceOneHorizWallThatIsNotOnTheBorder(Board_t* board);
void test_3_RemoveOneHorizWallThatIsNotOnTheBorder(Board_t* board);
void test_4_PlaceTwoConsecutiveHorizWalls(Board_t* board);


#endif // Header_qplugin_marinica_debug
