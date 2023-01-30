#include <stddef.h>
#include "tests.h"
#include "debug.h"

#define COUNT(A) (A == NULL ? 0 : (sizeof(A) / sizeof(A[0])))

#define DEFAULT_NULL_TILE_LINKS { 0, 0, N }, \
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

static void PlaceWalls(TestWall_t* walls, int8_t wallsCount)
{
    if (walls)
    {
        for (int i = 0 ; i < wallsCount; i++)
        {
            if (walls[i].wallOr == H)
            {
                PlaceHorizWallByMe({walls[i].x, walls[i].y});
            }
            else
            {
                PlaceVertWallByMe({walls[i].x, walls[i].y});
            }
        }
    }
}

static void UndoWalls(TestWall_t* walls, int8_t wallsCount)
{
    if (walls)
    {
        for (int i = 0 ; i < wallsCount; i++)
        {
            if (walls[i].wallOr == H)
            {
                UndoHorizWallByMe({walls[i].x, walls[i].y});
            }
            else
            {
                UndoVertWallByMe({walls[i].x, walls[i].y});
            }
        }
    }
}

static void CheckBoardStructure(Board_t* board, TestTileLink_t* tileLinksToTest, int8_t tileLinksCount,
                       TestPossibilityFlag_t* possibilityFlagsToCheck, int8_t possibilityFlagsCount)
{ 
    const char* errMsg;    

    // then, check tile links
    bool err = false;
    for (int i = 0; i < BOARD_SZ; i++)  // go through all tiles from the board and check their links against the given links
    {
        for (int j = 0; j < BOARD_SZ; j++)
        {
            TestTileLink_t n = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, N };
            bool found = false;
            for (int c = 0; c < tileLinksCount; c++)
            {
                if (n.x == tileLinksToTest[c].x && n.y == tileLinksToTest[c].y && n.dir == tileLinksToTest[c].dir) 
                {
                    found = true;
                }
            }
            if (found && board->tiles[i][j].north != NULL) // a link from the board is not NULL but it should be
            {
                err = true;
                errMsg = "Found a link that should be NULL but it's not!";
                debug_PrintTestErrorMsg(errMsg);
            }
            if (!found && board->tiles[i][j].north == NULL) // a link from the board is NULL but shouldn't be
            {
                err = true;
                errMsg = "Found a link that is NULL but shouldn't be!";
                debug_PrintTestErrorMsg(errMsg);
            }

            TestTileLink_t s = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, S };
            found = false;
            for (int c = 0; c < tileLinksCount; c++)
            {
                if (s.x == tileLinksToTest[c].x && s.y == tileLinksToTest[c].y && s.dir == tileLinksToTest[c].dir) 
                {
                    found = true;
                }
            }
            if (found && board->tiles[i][j].south != NULL) // a link from the board is not NULL but it should be
            {
                err = true;
                errMsg = "Found a link that should be NULL but it's not!";
                debug_PrintTestErrorMsg(errMsg);
            }
            if (!found && board->tiles[i][j].south == NULL) // a link from the board is NULL but shouldn't be
            {
                err = true;
                errMsg = "Found a link that is NULL but shouldn't be!";
                debug_PrintTestErrorMsg(errMsg);
            }

            TestTileLink_t e = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, E };
            found = false;
            for (int c = 0; c < tileLinksCount; c++)
            {
                if (e.x == tileLinksToTest[c].x && e.y == tileLinksToTest[c].y && e.dir == tileLinksToTest[c].dir) 
                {
                    found = true;
                }
            }
            if (found && board->tiles[i][j].east != NULL) // a link from the board is not NULL but it should be
            {
                err = true;
                errMsg = "Found a link that should be NULL but it's not!";
                debug_PrintTestErrorMsg(errMsg);
            }
            if (!found && board->tiles[i][j].east == NULL) // a link from the board is NULL but shouldn't be
            {
                err = true;
                errMsg = "Found a link that is NULL but shouldn't be!";
                debug_PrintTestErrorMsg(errMsg);
            }

            TestTileLink_t w = { board->tiles[i][j].pos.x, board->tiles[i][j].pos.y, W };
            found = false;
            for (int c = 0; c < tileLinksCount; c++)
            {
                if (w.x == tileLinksToTest[c].x && w.y == tileLinksToTest[c].y && w.dir == tileLinksToTest[c].dir) 
                {
                    found = true;
                }
            }
            if (found && board->tiles[i][j].west != NULL) // a link from the board is not NULL but it should be
            {
                err = true;
                errMsg = "Found a link that should be NULL but it's not!";
                debug_PrintTestErrorMsg(errMsg);
            }
            if (!found && board->tiles[i][j].west == NULL) // a link from the board is NULL but shouldn't be
            {
                err = true;
                errMsg = "Found a link that is NULL but shouldn't be!";
                debug_PrintTestErrorMsg(errMsg);
            }
        }
    }

    // possibility flags check
    for (int i = 0; i < BOARD_SZ - 1; i++)  // go through all H and V walls on the board and check their possibility flag against the given flags
    {                                       // if there are no given flags, check that all the flags are == POSSIBLE
        for (int j = 0; j < BOARD_SZ - 1; j++)
        {   
            TestPossibilityFlag_t flag;
            bool found;

            // H walls -------------------------------------------------------------------------------------------------------------------------------------
            flag = { H, board->hWalls[i][j].pos.x, board->hWalls[i][j].pos.y, (PossibilityFlag_t)board->hWalls[i][j].possibleFlag};
            found = false;

            if (possibilityFlagsToCheck)
            {
                for (int c = 0; c < possibilityFlagsCount; c++)
                {
                    if (flag.wallOr == possibilityFlagsToCheck[c].wallOr && flag.x == possibilityFlagsToCheck[c].x && flag.y == possibilityFlagsToCheck[c].y)  
                    {
                        found = true;
                        if (flag.possibilityFlag != possibilityFlagsToCheck[c].possibilityFlag) // the possibility flag of the wall on the board is different from the given one
                        {
                            err = true;
                            errMsg = "Found a PossibilityFlag that is different on the board than given in the test!";
                            debug_PrintTestErrorMsg(errMsg);
                        }
                    }
                }
            }

            if (!found && flag.possibilityFlag != POSSIBLE) // there are walls on the board with a possibility flag different from POSSIBLE, and there shouldn't be
            {
                err = true;
                errMsg = "Found a NOT POSSIBLE flag on the board that is not given in the test!";
                debug_PrintTestErrorMsg(errMsg);
            }

            // V walls -------------------------------------------------------------------------------------------------------------------------------------
            flag = { V, board->vWalls[i][j].pos.x, board->vWalls[i][j].pos.y, (PossibilityFlag_t)board->vWalls[i][j].possibleFlag};
            found = false;

            if (possibilityFlagsToCheck)
            {
                for (int c = 0; c < possibilityFlagsCount; c++)
                {
                    if (flag.wallOr == possibilityFlagsToCheck[c].wallOr && flag.x == possibilityFlagsToCheck[c].x && flag.y == possibilityFlagsToCheck[c].y)  
                    {
                        found = true;
                        if (flag.possibilityFlag != possibilityFlagsToCheck[c].possibilityFlag) // the possibility flag of the wall on the board is different from the given one
                        {
                            err = true;
                            errMsg = "Found a PossibilityFlag that is different on the board than given in the test!";
                            debug_PrintTestErrorMsg(errMsg);
                        }
                    }
                }
            }

            if (!found && flag.possibilityFlag != POSSIBLE) // there are walls on the board with a possibility flag different from POSSIBLE, and there shouldn't be
            {
                err = true;
                errMsg = "Found a NOT POSSIBLE flag on the board that is not given in the test!";
                debug_PrintTestErrorMsg(errMsg);
            }
        }
    }

    if (err)
    {
        debug_PrintTestFailed();
    }
    else
    {
        debug_PrintTestPassed();
    }
}

void test_1_CheckInitialBoardStructure(Board_t* board)
{
    debug_PrintTestMessage("Test 1.1:");

    // define some walls to place
    TestWall_t* wallsToPlace = NULL; // no walls placed for this test

    // define tile links that should be NULL
    TestTileLink_t tileLinksToTest[] =
    {
        DEFAULT_NULL_TILE_LINKS // check default tile links for this test (only links of border tiles should be NULL if no walls were placed)
    };

    // define some walls that should have the possibility flag NOT equal to POSSIBLE
    TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
    
    PlaceWalls(wallsToPlace, 0);
    
    CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);        
}


void test_2_PlaceOneHorizWallThatIsNotOnTheBorder(Board_t* board)
{
    debug_PrintTestMessage("Test 2.1:");

    // define some walls to place
    TestWall_t wallsToPlace[] =
    {
        { H, 1, 2 }
    };

    // define tile links that should be NULL
    TestTileLink_t tileLinksToTest[] =
    {
        DEFAULT_NULL_TILE_LINKS,
        { 1, 2, S },
        { 2, 2, N },
        { 1, 3, S },
        { 2, 3, N }
    };

    // define some walls that should be flagged as forbidden
    TestPossibilityFlag_t possibilityFlagsToCheck[] =
    {
        { H, 1, 2, FORBIDDEN_1X },
        { H, 1, 1, FORBIDDEN_1X },
        { H, 1, 3, FORBIDDEN_1X },
        { V, 1, 2, FORBIDDEN_1X }
    };
    
    PlaceWalls(wallsToPlace, COUNT(wallsToPlace));

    CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck)); 
}

void test_3_UndoLastWall(Board_t* board)
{
    debug_PrintTestMessage("Test 3.1:");

    // define some walls to undo
    TestWall_t wallsToUndo[] =
    {
        { H, 1, 2 }
    };

    // define tile links that should be NULL
    TestTileLink_t tileLinksToTest[] =
    {
        DEFAULT_NULL_TILE_LINKS // check default tile links for this test
    };

    // define some walls that should have the possibility flag NOT equal to POSSIBLE
    TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
    
    UndoWalls(wallsToUndo, COUNT(wallsToUndo));

    CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);   
}

void test_4_PlaceTwoConsecutiveHorizWalls(Board_t* board)
{
    debug_PrintTestMessage("Test 4.1:");

    // define some walls to place
    TestWall_t wallsToPlace[] =
    {
        { H, 1, 2 },
        { H, 1, 4 }
    };

    // define tile links that should be NULL
    TestTileLink_t tileLinksToTest[] =
    {
        DEFAULT_NULL_TILE_LINKS,
        { 1, 2, S },
        { 2, 2, N },
        { 1, 3, S },
        { 2, 3, N },
        { 1, 4, S },
        { 2, 4, N },
        { 1, 5, S },
        { 2, 5, N }
    };

    // define some walls that should be flagged as forbidden
    TestPossibilityFlag_t possibilityFlagsToCheck[] =
    {
        { H, 1, 2, FORBIDDEN_1X },
        { H, 1, 1, FORBIDDEN_1X },
        { H, 1, 3, FORBIDDEN_2X }, // This is forbidden by both H 1 2 and H 1 4
        { V, 1, 2, FORBIDDEN_1X },
        { H, 1, 4, FORBIDDEN_1X },
        { H, 1, 5, FORBIDDEN_1X },
        { V, 1, 4, FORBIDDEN_1X },
    };

    PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
    
    CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
}


void test_5_UndoLastTwoWallsOneByOne(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 5.1:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 4 } // Undo last wall from previous test (Undoing is done in the reverse order compared to placing)
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 1, 2, S },
            { 2, 2, N },
            { 1, 3, S },
            { 2, 3, N }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 1, 2, FORBIDDEN_1X },
            { H, 1, 1, FORBIDDEN_1X },
            { H, 1, 3, FORBIDDEN_1X },
            { V, 1, 2, FORBIDDEN_1X }
        };
        
        UndoWalls(wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck)); 
    }

    {
        debug_PrintTestMessage("Test 5.2:");
        
        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 2 } // Undo the first wall from previous test (Undoing is done in the reverse order compared to placing)
        };
        
        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the possibility flag NOT equal to POSSIBLE
        TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
        
        UndoWalls(wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);
    }
}

void test_6_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 6.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 1, 2 },
            { H, 1, 4 },
            { V, 1, 3 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 1, 2, S },
            { 2, 2, N },
            { 1, 3, S },
            { 2, 3, N },
            { 1, 4, S },
            { 2, 4, N },
            { 1, 5, S },
            { 2, 5, N },
            { 1, 3, E },
            { 1, 4, W },
            { 2, 3, E },
            { 2, 4, W }

        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 1, 2, FORBIDDEN_1X },
            { H, 1, 1, FORBIDDEN_1X },
            { H, 1, 3, FORBIDDEN_3X }, // This is forbidden by H 1 2 and H 1 4 and V 1 3
            { V, 1, 2, FORBIDDEN_1X },
            { H, 1, 4, FORBIDDEN_1X },
            { H, 1, 5, FORBIDDEN_1X },
            { V, 1, 4, FORBIDDEN_1X },
            { V, 1, 3, FORBIDDEN_1X },
            { V, 0, 3, FORBIDDEN_1X },
            { V, 2, 3, FORBIDDEN_1X }
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 6.2:");

        // undo last wall from 6.1
        TestWall_t wallsToUndo[] =
        {
            { V, 1, 3 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 1, 2, S },
            { 2, 2, N },
            { 1, 3, S },
            { 2, 3, N },
            { 1, 4, S },
            { 2, 4, N },
            { 1, 5, S },
            { 2, 5, N }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 1, 2, FORBIDDEN_1X },
            { H, 1, 1, FORBIDDEN_1X },
            { H, 1, 3, FORBIDDEN_2X }, // This is forbidden by both H 1 2 and H 1 4
            { V, 1, 2, FORBIDDEN_1X },
            { H, 1, 4, FORBIDDEN_1X },
            { H, 1, 5, FORBIDDEN_1X },
            { V, 1, 4, FORBIDDEN_1X },
        };

        UndoWalls(wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 6.3:");
        
        // undo remaining walls
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 2 },
            { H, 1, 4 }
        };
        
        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the possibility flag NOT equal to POSSIBLE
        TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
        
        UndoWalls(wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);
    }
}


void test_7_Place2VertWallsAndOneHorizWallAndThenUndoAll(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 7.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 7, 7 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { V, 7, 7, FORBIDDEN_1X },
            { V, 6, 7, FORBIDDEN_1X },
            { H, 7, 7, FORBIDDEN_1X }
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.2:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 6, 6 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W },
            { 6, 6, E },
            { 6, 7, W },
            { 7, 6, E },
            { 7, 7, W }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { V, 7, 7, FORBIDDEN_1X },
            { V, 6, 7, FORBIDDEN_1X },
            { H, 7, 7, FORBIDDEN_1X },
            { V, 6, 6, FORBIDDEN_1X },
            { V, 7, 6, FORBIDDEN_1X },
            { H, 6, 6, FORBIDDEN_1X },
            { V, 5, 6, FORBIDDEN_1X },
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.3:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 6 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W },
            { 6, 6, E },
            { 6, 7, W },
            { 7, 6, E },
            { 7, 7, W },
            { 7, 6, S },
            { 8, 6, N },
            { 7, 7, S },
            { 8, 7, N }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { V, 7, 7, FORBIDDEN_1X },
            { V, 6, 7, FORBIDDEN_1X },
            { H, 7, 7, FORBIDDEN_2X },
            { V, 6, 6, FORBIDDEN_1X },
            { V, 7, 6, FORBIDDEN_2X },
            { H, 6, 6, FORBIDDEN_1X },
            { V, 5, 6, FORBIDDEN_1X },
            { H, 7, 6, FORBIDDEN_1X },
            { H, 7, 5, FORBIDDEN_1X },
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.4:");

        // Undo wall from 7.3
        TestWall_t wallsToUndo[] =
        {
            { H, 7, 6 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W },
            { 6, 6, E },
            { 6, 7, W },
            { 7, 6, E },
            { 7, 7, W }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { V, 7, 7, FORBIDDEN_1X },
            { V, 6, 7, FORBIDDEN_1X },
            { H, 7, 7, FORBIDDEN_1X },
            { V, 6, 6, FORBIDDEN_1X },
            { V, 7, 6, FORBIDDEN_1X },
            { H, 6, 6, FORBIDDEN_1X },
            { V, 5, 6, FORBIDDEN_1X },
        };

        UndoWalls(wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.5:");

        // Undo wall from 7.2
        TestWall_t wallsToUndo[] =
        {
            { V, 6, 6 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { V, 7, 7, FORBIDDEN_1X },
            { V, 6, 7, FORBIDDEN_1X },
            { H, 7, 7, FORBIDDEN_1X }
        };

        UndoWalls(wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.6:");

        // undo wall from 7.1
        TestWall_t wallsToUndo[] =
        {
            { V, 7, 7 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the possibility flag NOT equal to POSSIBLE
        TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
        
        UndoWalls(wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);
    }
}

void test_8_PlaceAndUndoGroupsOf3Walls(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 8.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 0 },
            { V, 6, 0 },
            { H, 5, 1 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 0, S },
            { 8, 0, N },
            { 7, 1, S },
            { 8, 1, N },
            { 6, 0, E },
            { 6, 1, W },
            { 7, 0, E },
            { 7, 1, W },
            { 5, 1, S },
            { 6, 1, N },
            { 5, 2, S },
            { 6, 2, N }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 7, 0, FORBIDDEN_1X },
            { H, 7, 1, FORBIDDEN_1X },
            { V, 7, 0, FORBIDDEN_2X },
            { V, 5, 0, FORBIDDEN_1X },
            { V, 6, 0, FORBIDDEN_1X },
            { H, 5, 0, FORBIDDEN_1X },
            { H, 5, 1, FORBIDDEN_1X },
            { H, 5, 2, FORBIDDEN_1X },
            { V, 5, 1, FORBIDDEN_1X },
            { H, 6, 0, FORBIDDEN_1X }
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.2:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 4, 3 },
            { H, 5, 3 },
            { V, 6, 2 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 0, S },
            { 8, 0, N },
            { 7, 1, S },
            { 8, 1, N },
            { 6, 0, E },
            { 6, 1, W },
            { 7, 0, E },
            { 7, 1, W },
            { 5, 1, S },
            { 6, 1, N },
            { 5, 2, S },
            { 6, 2, N },
            { 4, 3, S },
            { 5, 3, N },
            { 4, 4, S },
            { 5, 4, N },
            { 5, 3, S },
            { 6, 3, N },
            { 5, 4, S },
            { 6, 4, N },
            { 6, 2, E },
            { 6, 3, W },
            { 7, 2, E },
            { 7, 3, W }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 7, 0, FORBIDDEN_1X },
            { H, 7, 1, FORBIDDEN_1X },
            { V, 7, 0, FORBIDDEN_2X },
            { V, 5, 0, FORBIDDEN_1X },
            { V, 6, 0, FORBIDDEN_1X },
            { H, 5, 0, FORBIDDEN_1X },
            { H, 5, 1, FORBIDDEN_1X },
            { H, 5, 2, FORBIDDEN_2X },
            { V, 5, 1, FORBIDDEN_1X },
            { H, 6, 0, FORBIDDEN_1X },
            { H, 4, 3, FORBIDDEN_1X },
            { V, 4, 3, FORBIDDEN_1X },
            { H, 4, 2, FORBIDDEN_1X },
            { H, 4, 4, FORBIDDEN_1X },
            { H, 5, 3, FORBIDDEN_1X },
            { H, 5, 4, FORBIDDEN_1X },
            { V, 5, 3, FORBIDDEN_1X },
            { V, 7, 2, FORBIDDEN_1X },
            { V, 5, 2, FORBIDDEN_1X },
            { H, 6, 2, FORBIDDEN_1X },
            { V, 6, 2, FORBIDDEN_1X }
        };

        PlaceWalls(wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.3:");

        // undo walls from 8.2 in reverse order
        TestWall_t wallsToUndo[] =
        {
            { V, 6, 2 },
            { H, 5, 3 },
            { H, 4, 3 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 0, S },
            { 8, 0, N },
            { 7, 1, S },
            { 8, 1, N },
            { 6, 0, E },
            { 6, 1, W },
            { 7, 0, E },
            { 7, 1, W },
            { 5, 1, S },
            { 6, 1, N },
            { 5, 2, S },
            { 6, 2, N }
        };

        // define some walls that should be flagged as forbidden
        TestPossibilityFlag_t possibilityFlagsToCheck[] =
        {
            { H, 7, 0, FORBIDDEN_1X },
            { H, 7, 1, FORBIDDEN_1X },
            { V, 7, 0, FORBIDDEN_2X },
            { V, 5, 0, FORBIDDEN_1X },
            { V, 6, 0, FORBIDDEN_1X },
            { H, 5, 0, FORBIDDEN_1X },
            { H, 5, 1, FORBIDDEN_1X },
            { H, 5, 2, FORBIDDEN_1X },
            { V, 5, 1, FORBIDDEN_1X },
            { H, 6, 0, FORBIDDEN_1X }
        };

        UndoWalls(wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, COUNT(possibilityFlagsToCheck));

    }

    {
        debug_PrintTestMessage("Test 8.4:");

        // undo walls from 8.1 in reverse order
        TestWall_t wallsToUndo[] =
        {
            { H, 5, 1 },
            { V, 6, 0 },
            { H, 7, 0 }
        };

        // define tile links that should be NULL
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the possibility flag NOT equal to POSSIBLE
        TestPossibilityFlag_t* possibilityFlagsToCheck = NULL; // all walls should be possible for this test
        
        UndoWalls(wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), possibilityFlagsToCheck, 0);

    }
}