#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include "tests.h"
#include "debug.h"
#include "min_path.h"

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

static void PlaceWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount)
{
    if (walls)
    {
        for (int i = 0 ; i < wallsCount; i++)
        {
            PlaceWall(ME, &(board->walls[walls[i].ori][walls[i].x][walls[i].y]));
        }
    }
}

static void UndoWalls(Board_t* board, TestWall_t* walls, int8_t wallsCount)
{
    if (walls)
    {
        for (int i = 0 ; i < wallsCount; i++)
        {
            UndoWall(ME, &(board->walls[walls[i].ori][walls[i].x][walls[i].y]));
        }
    }
}

static void CheckBoardStructure(Board_t* board, TestTileLink_t* tileLinksToTest, int8_t tileLinksCount,
                       TestWallPermission_t* permissionsToCheck, int8_t permissionsToCheckCount)
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

    // permissions check
    for (int i = 0; i < BOARD_SZ - 1; i++)  // go through all H and V walls on the board and check their permission level against the given level
    {                                       // if there are no given permissions, check that all the levels are WALL_PERMITTED
        for (int j = 0; j < BOARD_SZ - 1; j++)
        {   
            TestWallPermission_t toCheck;
            bool found;

            // H walls -------------------------------------------------------------------------------------------------------------------------------------
            toCheck = { H, board->walls[H][i][j].pos.x, board->walls[H][i][j].pos.y, board->walls[H][i][j].permission};
            found = false;

            if (permissionsToCheck)
            {
                for (int c = 0; c < permissionsToCheckCount; c++)
                {
                    if (toCheck.ori == permissionsToCheck[c].ori && toCheck.x == permissionsToCheck[c].x && toCheck.y == permissionsToCheck[c].y)  
                    {
                        found = true;
                        if (toCheck.permission != permissionsToCheck[c].permission) // the permission level of the wall on the board is different from the given one
                        {
                            err = true;
                            errMsg = "Found a permission level that is different on the board than given in the test!";
                            debug_PrintTestErrorMsg(errMsg);
                        }
                    }
                }
            }

            if (!found && toCheck.permission != WALL_PERMITTED) // there are walls on the board with a permission level different from WALL_PERMITTED, and there shouldn't be
            {
                err = true;
                errMsg = "Found a FORBIDDEN permission level on the board that is not given in the test!";
                debug_PrintTestErrorMsg(errMsg);
            }

            // V walls -------------------------------------------------------------------------------------------------------------------------------------
            toCheck = { V, board->walls[V][i][j].pos.x, board->walls[V][i][j].pos.y, board->walls[V][i][j].permission};
            found = false;

            if (permissionsToCheck)
            {
                for (int c = 0; c < permissionsToCheckCount; c++)
                {
                    if (toCheck.ori == permissionsToCheck[c].ori && toCheck.x == permissionsToCheck[c].x && toCheck.y == permissionsToCheck[c].y)  
                    {
                        found = true;
                        if (toCheck.permission != permissionsToCheck[c].permission) // the permission level of the wall on the board is different from the given one
                        {
                            err = true;
                            errMsg = "Found a permission level that is different on the board than given in the test!";
                            debug_PrintTestErrorMsg(errMsg);
                        }
                    }
                }
            }

            if (!found && toCheck.permission != WALL_PERMITTED) // there are walls on the board with a permission level different from WALL_PERMITTED, and there shouldn't be
            {
                err = true;
                errMsg = "Found a FORBIDDEN permission level on the board that is not given in the test!";
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

    // define tile links that should be NULL after walls are placed
    TestTileLink_t tileLinksToTest[] =
    {
        DEFAULT_NULL_TILE_LINKS // check default tile links for this test (only links of border tiles should be NULL if no walls were placed)
    };

    // define some walls that should have the permission level NOT equal to WALL_PERMITTED
    TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
    
    PlaceWalls(board, wallsToPlace, 0);
    
    CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);        
}


void test_2_PlaceThenUndoOneHorizWallThatIsNotOnTheBorder(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 2.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 1, 2 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 1, 2, S },
            { 2, 2, N },
            { 1, 3, S },
            { 2, 3, N }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 2.2:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 2 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}

void test_3_PlaceThenUndoOneVertWallThatIsNotOnTheBorder(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 3.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 6, 2 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 6, 2, E },
            { 6, 3, W },
            { 7, 2, E },
            { 7, 3, W }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 5, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 3.2:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { V, 6, 2 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}


void test_4_PlaceThenUndoOneHorizWallThatIsOnTheBorder(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 4.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 0 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 0, S },
            { 8, 0, N },
            { 7, 1, S },
            { 8, 1, N }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {            
            { H, 7, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 4.2:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 7, 0 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
    {
        debug_PrintTestMessage("Test 4.3:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, S },
            { 8, 7, N },
            { 7, 8, S },
            { 8, 8, N }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {            
            { H, 7, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 4.4:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 7, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}

void test_5_PlaceThenUndoOneVertWallThatIsOnTheBorder(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 5.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 0, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 0, 7, E },
            { 0, 8, W },
            { 1, 7, E },
            { 1, 8, W }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 1, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 0, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 0, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 5.2:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { V, 0, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
    {
        debug_PrintTestMessage("Test 5.3:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 0, 0 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 0, 0, E },
            { 0, 1, W },
            { 1, 0, E },
            { 1, 1, W }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 1, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 0, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 0, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };
        
        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 5.4:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { V, 0, 0 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}


void test_6_PlaceTwoConsecutiveHorizWallsAndUndoThem(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 6.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 1, 2 },
            { H, 1, 4 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 3, WALL_FORBIDDEN_BY_2_OTHER_WALLS }, // This is forbidden by both H 1 2 and H 1 4
            { V, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 5, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }
    {
        debug_PrintTestMessage("Test 6.2:");

        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 4 } // Undo last wall from previous test (Undoing is done in the reverse order compared to placing)
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 1, 2, S },
            { 2, 2, N },
            { 1, 3, S },
            { 2, 3, N }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck)); 
    }

    {
        debug_PrintTestMessage("Test 6.3:");
        
        // define some walls to undo
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 2 } // Undo the first wall from previous test (Undoing is done in the reverse order compared to placing)
        };
        
        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}


void test_7_Place2HorizWallsAndOneVertWallBetweenThemAndThenUndoAll(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 7.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 1, 2 },
            { H, 1, 4 },
            { V, 1, 3 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 3, WALL_FORBIDDEN_BY_3_OTHER_WALLS }, // This is forbidden by H 1 2 and H 1 4 and V 1 3
            { V, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 5, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 0, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 2, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.2:");

        // undo last wall
        TestWall_t wallsToUndo[] =
        {
            { V, 1, 3 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 3, WALL_FORBIDDEN_BY_2_OTHER_WALLS }, // This is forbidden by both H 1 2 and H 1 4
            { V, 1, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 1, 5, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 1, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };

        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 7.3:");
        
        // undo remaining walls
        TestWall_t wallsToUndo[] =
        {
            { H, 1, 2 },
            { H, 1, 4 }
        };
        
        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}


void test_8_Place2VertWallsAndOneHorizWallAndThenUndoAll(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 8.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 7, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.2:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { V, 6, 6 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.3:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 6 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { V, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 6, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { H, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 5, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.4:");

        // Undo last wall
        TestWall_t wallsToUndo[] =
        {
            { H, 7, 6 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 6, WALL_FORBIDDEN_BY_1_OTHER_WALL },
        };

        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.5:");

        // Undo wall that was placed second
        TestWall_t wallsToUndo[] =
        {
            { V, 6, 6 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS,
            { 7, 7, E },
            { 7, 8, W },
            { 8, 7, E },
            { 8, 8, W }
        };

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { V, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 7, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 8.6:");

        // undo first wall
        TestWall_t wallsToUndo[] =
        {
            { V, 7, 7 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);
    }
}

void test_9_PlaceAndUndoGroupsOf3Walls(Board_t* board)
{
    {
        debug_PrintTestMessage("Test 9.1:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 7, 0 },
            { V, 6, 0 },
            { H, 5, 1 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 7, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 0, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { V, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 9.2:");

        // define some walls to place
        TestWall_t wallsToPlace[] =
        {
            { H, 4, 3 },
            { H, 5, 3 },
            { V, 6, 2 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 7, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 0, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { V, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 2, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { V, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 4, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 4, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 4, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 4, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 4, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 3, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        PlaceWalls(board, wallsToPlace, COUNT(wallsToPlace));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));
    }

    {
        debug_PrintTestMessage("Test 9.3:");

        // undo walls from 9.2 in reverse order
        TestWall_t wallsToUndo[] =
        {
            { V, 6, 2 },
            { H, 5, 3 },
            { H, 4, 3 }
        };

        // define tile links that should be NULL after walls are placed
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

        // define some walls that should be flagged as forbidden after the placement of walls above
        TestWallPermission_t permissionsToCheck[] =
        {
            { H, 7, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 7, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 7, 0, WALL_FORBIDDEN_BY_2_OTHER_WALLS },
            { V, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 5, 2, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { V, 5, 1, WALL_FORBIDDEN_BY_1_OTHER_WALL },
            { H, 6, 0, WALL_FORBIDDEN_BY_1_OTHER_WALL }
        };

        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));
        
        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, COUNT(permissionsToCheck));

    }

    {
        debug_PrintTestMessage("Test 9.4:");

        // undo walls from 9.1 in reverse order
        TestWall_t wallsToUndo[] =
        {
            { H, 5, 1 },
            { V, 6, 0 },
            { H, 7, 0 }
        };

        // define tile links that should be NULL after walls are placed
        TestTileLink_t tileLinksToTest[] =
        {
            DEFAULT_NULL_TILE_LINKS // check default tile links for this test
        };

        // define some walls that should have the permission level NOT equal to WALL_PERMITTED
        TestWallPermission_t* permissionsToCheck = NULL; // all walls should be permitted for this test
        
        UndoWalls(board, wallsToUndo, COUNT(wallsToUndo));

        CheckBoardStructure(board, tileLinksToTest, COUNT(tileLinksToTest), permissionsToCheck, 0);

    }
}

static void StringToMap(const char* stringInput, char* mapOutput, int8_t* myPosX, int8_t* myPosY, int8_t* oppPosX, int8_t* oppPosY)
{
    int n = 0, i = 0, j = 0;

    // copy const string to char array for tokenizing
    char mapString[1000];
    strcpy(mapString, stringInput);
    mapString[strlen(stringInput) + 1] = 0;

    char* token = strtok(mapString, ",");
    n++;

    while (token != NULL) 
    {
        mapOutput[i * 17 + j] = token[0];

        if (mapOutput[i * 17 + j] == '0') 
        {
            *myPosX = i;
            *myPosY = j;
        }

        if (mapOutput[i * 17 + j] == '1')
        {
            *oppPosX = i;
            *oppPosY = j;
        }

        j++;

        if (n % 17 == 0)
        {
            i++;
            j = 0;
        }

        token = strtok(NULL, ",");
        n++;
    }
}


static int TestGetMinPathToTargetTile(char* map, int playerX, int playerY, int targetX, int targetY)
{
    int distances[17][17] = { 0 };
    distances[playerX][playerY] = 1;
    int scan = 0;
    bool noPath;


    while (1)
    {
        int i, j;
        scan++;
        noPath = true;

        for (i = 0; i < 17; i += 2)
        {
            for (j = 0; j < 17; j += 2)
            {
                if (distances[i][j] == 0)
                {   
                    if (i - 2 >= 0 && map[(i - 1) * 17 + j] != '=' && distances[i - 2][j] == scan)
                    {
                        distances[i][j] = distances[i - 2][j] + 1;
                        noPath = false;
                        if (targetX == i && targetY == j)
                        {
                            return (distances[i][j] - 1);
                        }
                    }
                    else if (i + 2 <= 16 && map[(i + 1) * 17 + j] != '=' && distances[i + 2][j] == scan)
                    {
                        distances[i][j] = distances[i + 2][j] + 1;
                        noPath = false;
                        if (targetX == i && targetY == j)
                        {
                            return (distances[i][j] - 1);
                        }
                    }
                    else if (j - 2 >= 0 && map[i * 17 + (j - 1)] != '|' && distances[i][j - 2] == scan)
                    {
                        distances[i][j] = distances[i][j - 2] + 1;
                        noPath = false;
                        if (targetX == i && targetY == j)
                        {
                            return (distances[i][j] - 1);
                        }
                    }
                    else if (j + 2 <= 16 && map[i* 17 + (j + 1)] != '|' && distances[i][j + 2] == scan)
                    {
                        distances[i][j] = distances[i][j + 2] + 1;
                        noPath = false;
                        if (targetX == i && targetY == j)
                        {
                            return (distances[i][j] - 1);
                        }
                    }          
                }
            }
        }

        if (noPath)
        {
            return 0xFFFF;
        }
    }
}


static int TestGetMinPathForMe(char* map, int myX, int myY)
{
    int min = 0xFFFF;

    for (int j = 0; j < 17; j += 2)
    {
        int tempMin = TestGetMinPathToTargetTile(map, myX, myY, 0, j);
        if (min > tempMin)
        {
            min = tempMin;
        }
    }

    return min;
}

static int TestGetMinPathForOpp(char* map, int oppX, int oppY)
{
    int min = 0xFFFF;

    for (int j = 0; j < 17; j += 2)
    {
        int tempMin = TestGetMinPathToTargetTile(map, oppX, oppY, 16, j);
        if (min > tempMin)
        {
            min = tempMin;
        }
    }

    return min;
}

static void MapToBoard(char* map, Board_t* board, int8_t myMapPosX, int8_t myMapPosY, int8_t oppMapPosX, int8_t oppMapPosY)
{
    for (int i = 0; i < 17; i += 2)
    {
        for (int j = 0; j < 17; j += 2)
        {
            Tile_t* tile = &(board->tiles[i / 2][j / 2]);

            if ((j <= 14) && map[i * 17 + (j + 1)] == '|') // there is a wall to the east of the current tile
            {
                // remove horizontal links
                tile->east->west = NULL;
                tile->east = NULL;                
            }

            if ((i <= 14) && map[(i + 1) * 17 + j] == '=') // there is a wall to the south of the current tile
            {
                // remove vertical links
                tile->south->north = NULL;
                tile->south = NULL;                
            }
        }
    }

    board->playerPos[ME] = { (int8_t)(myMapPosX / 2), (int8_t)(myMapPosY / 2)};
    board->playerPos[OPPONENT] = { (int8_t)(oppMapPosX / 2), (int8_t)(oppMapPosY / 2)};
}

static bool IsMinPathAndPossibleMovesTestPassed(const char* stringInput, const char* possibleMovesMeInput, const char* possibleMovesOppInput)
{
    int8_t myMapPosX;
    int8_t myMapPosY;
    int8_t oppMapPosX;
    int8_t oppMapPosY;
    
    // convert input to a 17 x 17 char map and get player positions
    char map[17][17] = { 0 };
    StringToMap(stringInput, (char*)map, &myMapPosX, &myMapPosY, &oppMapPosX, &oppMapPosY);

    // save existing board
    Board_t boardBkp;
    memcpy(&boardBkp, GetBoard(), sizeof(Board_t));

    // replace plugin board with a new test board
    Board_t testBoard;
    memcpy(GetBoard(), &testBoard, sizeof(Board_t));

    // init new board
    InitBoard();

    // Update the new board with the given test configuration
    MapToBoard((char*)map, GetBoard(), myMapPosX, myMapPosY, oppMapPosX, oppMapPosY);

    // Get min path the test way
    debug_PrintTestMessage("  Min path Test:");
    int minPathMeTest = TestGetMinPathForMe((char*)map, myMapPosX, myMapPosY);
    int minPathOppTest = TestGetMinPathForOpp((char*)map, oppMapPosX, oppMapPosY);
    debug_PrintMinPaths(minPathMeTest, minPathOppTest);

    // Get min path the plugin way
    debug_PrintTestMessage("  Min path Plugin:");
    int minPathMePlugin = FindMinPathLen(ME);
    int minPathOppPlugin = FindMinPathLen(OPPONENT);
    debug_PrintMinPaths(minPathMePlugin, minPathOppPlugin);

    // Update possible moves for both
    UpdatePossibleMoves(ME);
    UpdatePossibleMoves(OPPONENT);

    bool ret = true;

    // Print possible moves from test
    debug_PrintTestMessage("  Possible Moves Test:");
    debug_PrintTestMessage(possibleMovesMeInput);
    debug_PrintTestMessage(possibleMovesOppInput);

    // Print possible moves from plugin
    debug_PrintTestMessage("  Possible Moves Plugin:");

    if (strcmp(debug_PrintMyPossibleMoves(GetBoard()), possibleMovesMeInput) != 0 ||
         strcmp(debug_PrintOppPossibleMoves(GetBoard()), possibleMovesOppInput) != 0 ||
          (minPathMePlugin != minPathMeTest) || 
            (minPathOppPlugin != minPathOppTest))
            {
                ret = false;
            }

    // restore plugin board
    memcpy(GetBoard(), &boardBkp, sizeof(Board_t));

    // report
    if (ret)
    {
        return true;
    }

    return false;
}

void test_10_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 10: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,., ,.,1,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,.,0,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-E],[M-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-E],[M-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}

void test_11_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 11: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,.,1,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,.,0,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-E],[M-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[M-E],[M-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_12_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 12: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,|, ,., ,|, ,., ,., ,., ,"
    ",.,.,.,.,.,|,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,., ,., ,|, ,., ,|, ,., ,., ,., ,"
    ",.,.,.,.,.,.,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,.,1,|, ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,.,0,|, ,., ,., ,., ,"
    ",.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-W],[J-N-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-W],[J-S-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}

void test_13_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 13: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,|,0,.,1,., ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[J-E],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[M-E],[J-N-W],[J-S-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_14_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 14: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,=,=,=,.,=,=,=,"
    ", ,., ,., ,|,0,.,1,., ,., ,., ,., ,"
    ",.,.,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [J-E],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-E],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_15_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 15: Min path and possible moves");

    const char* stringInput = 
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",=,=,=,.,=,=,=,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,.,0,.,1,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,=,=,=,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,|, ,., ,., ,., ,"
    ",.,|,.,.,.,|,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-S],[M-W],[J-N-E],[J-S-E],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_16_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 16: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,.,0,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,.,1,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,=,=,=,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-E],[M-W],[J-S-E],[J-S-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-E],[M-W],[J-N-E],[J-N-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}

void test_17_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 17: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,.,0,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,.,1,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-W],[J-S],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-E],[M-W],[J-N],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}

void test_18_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 18: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,.,0,.,1,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-W],[J-N-E],[J-S-E],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_19_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 19: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ",1,.,0,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-E],[J-N-W],[J-S-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-S],[J-E],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_20_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 20: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,.,1,.,0,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-N],[M-S],[M-E],[J-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-S],[M-W],[J-E],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_21_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 21: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,., ,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,.,1,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,.,0,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-E],[M-W],[J-N],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-N],[M-E],[M-W],[J-S-E],[J-S-W],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}


void test_22_MinPathAndPossibleMoves(void)
{
    debug_PrintTestMessage("Test 22: Min path and possible moves");

    const char* stringInput = 
    ", ,|, ,., ,., ,., ,., ,., ,.,1,., ,"
    ",.,|,=,=,=,.,=,=,=,.,=,=,=,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,., ,.,0,., ,"
    ",.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,|, ,|, ,., ,., ,"
    ",.,|,.,.,=,=,=,.,.,.,.,|,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,|, ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,=,=,=,.,.,"
    ", ,., ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,|,=,=,=,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,|, ,., ,., ,., ,., ,., ,"
    ",.,|,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,|, ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,"
    ",.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,.,"
    ", ,., ,., ,., ,., ,., ,., ,., ,., ,";

    const char* possibleMovesMeInput = "  My possible moves: [M-S],[M-E],[M-W],[J-N-E],[J-N-W],";
    const char* possibleMovesOppInput = "  Opp possible moves: [M-E],[M-W],[J-S],";
    

    if (IsMinPathAndPossibleMovesTestPassed(stringInput, possibleMovesMeInput, possibleMovesOppInput))
    {
        debug_PrintTestPassed();
    }
    else
    {
        debug_PrintTestFailed();
    }
}