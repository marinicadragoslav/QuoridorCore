#include <stdio.h>
#include <string.h>
#include "QcoreUtil.h"
#include "debug.h"
#include "min_path.h"
#include "drma_player.h"

#define LOGGED_LINE_MAX_LEN  120

namespace qplugin_drma 
{
    static const char * const DOM = "qplugin_drma";
    static char buff[LOGGED_LINE_MAX_LEN];

    static void ClearBuff(void)
    {
        memset(buff, 0, sizeof(buff));
    }

    static const char* debug_ConvertMoveIDToString(MoveID_t moveID)
    {   
        switch (moveID)
        {
            case MOVE_NORTH:        return "M-N";
            case MOVE_SOUTH:        return "M-S";
            case MOVE_WEST:         return "M-W";
            case MOVE_EAST:         return "M-E";
            case JUMP_NORTH:        return "J-N";
            case JUMP_SOUTH:        return "J-S";
            case JUMP_EAST:         return "J-E";
            case JUMP_WEST:         return "J-W";
            case JUMP_NORTH_EAST:   return "J-N-E";
            case JUMP_NORTH_WEST:   return "J-N-W";
            case JUMP_SOUTH_EAST:   return "J-S-E";
            case JUMP_SOUTH_WEST:   return "J-S-W";
            default:                return "F*ck";
        }
    }

    #if (RUN_TESTS)
        void debug_PrintTestMessage(const char* msg)
        {
            LOG_WARN(DOM) << msg;
        }

        void debug_PrintTestPassed(void)
        {
            LOG_WARN(DOM) << "TEST PASSED!";
        }

        void debug_PrintTestFailed(void)
        {
            LOG_ERROR(DOM) << "TEST FAILED!";
        }

        void debug_PrintTestErrorMsg(const char* errMsg)
        {
            LOG_ERROR(DOM) << errMsg;
        }

        void debug_PrintTestMinPaths(int minPathMe, int minPathOpp)
        {
            LOG_INFO(DOM) << "  Me: " << minPathMe << ", Opponent: " << minPathOpp;
        }
    #endif

    char* debug_PrintMyPossibleMoves(Board_t* board)
    {    
        ClearBuff();
        sprintf(buff + strlen(buff), "  My possible moves: ");
        
        for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
        {
            if (board->moves[ME][i].isPossible)
            {
                sprintf(buff + strlen(buff), "[%s],", debug_ConvertMoveIDToString((MoveID_t)i));
            }
        }
        LOG_INFO(DOM) << buff;
        return buff;
    }

    char* debug_PrintOppPossibleMoves(Board_t* board)
    {    
        ClearBuff();
        sprintf(buff + strlen(buff), "  Opp possible moves: ");
        
        for (int i = MOVE_FIRST; i <= MOVE_LAST; i++)
        {
            if (board->moves[OPPONENT][i].isPossible)
            {
                sprintf(buff + strlen(buff), "[%s],", debug_ConvertMoveIDToString((MoveID_t)i));
            }
        }
        LOG_INFO(DOM) << buff;
        return buff;
    }    

    void debug_PrintPlay(Play_t play)
    {
        ClearBuff();
        sprintf(buff + strlen(buff), "  Best Play: ");
        if (play.action == MAKE_MOVE)
        {
            sprintf(buff + strlen(buff), "  Make move [%s]", debug_ConvertMoveIDToString(play.moveID));
        }
        else if (play.action == PLACE_WALL)
        {
            sprintf(buff + strlen(buff), "  Place wall: ");
            if (play.wall->orientation == H)
            {
                sprintf(buff + strlen(buff), "H[%d, %d],", play.wall->pos.x, play.wall->pos.y);
            }
            else
            {
                sprintf(buff + strlen(buff), "V[%d, %d],", play.wall->pos.x, play.wall->pos.y);
            }
        }
        else
        {
            sprintf(buff + strlen(buff), "  NO BEST PLAY FOUND");
        }
        LOG_INFO(DOM) << buff;
    }

    void debug_PrintBoard(Board_t* board)
    {
        char tiles[BOARD_SZ * BOARD_SZ] = { 0 };
        char vertw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };
        char horizw[BOARD_SZ * (BOARD_SZ - 1)] = { 0 };

        uint8_t ti = 0; 
        uint8_t hi = 0;
        uint8_t vi = 0;

        for (uint8_t i = 0; i < BOARD_SZ; i++)
        {
            for (uint8_t j = 0; j < BOARD_SZ; j++)
            {
                // populate vertical wall array
                if (j < BOARD_SZ - 1) 
                {
                    vertw[vi++] = ((board->tiles[i][j].east == NULL) ? '|' : ' ');
                }

                // populate horiz wall array
                if (i < BOARD_SZ - 1) 
                {
                    horizw[hi++] = ((board->tiles[i][j].south == NULL) ? '-' : ' ');
                }

                // populate tiles array
                if (i == board->playerPos[ME].x && j == board->playerPos[ME].y) 
                { 
                    tiles[ti++] = 'M'; // 'M' for my postion
                    continue;
                }
                if (i == board->playerPos[OPPONENT].x && j == board->playerPos[OPPONENT].y) 
                { 
                    tiles[ti++] = 'O'; // 'O' for opponent's position
                    continue;  
                }
                #if (SHOW_MIN_PATH_ON_LOGGED_BOARD)
                    if (board->isOnMinPath[i][j])
                    { 
                        tiles[ti++] = '*'; // '*' if tile is part of my min path
                        continue; 
                    }
                #endif
                tiles[ti++] = ' ';
            }
        }
        
        ti = 0;
        hi = 0;
        vi = 0;

        LOG_INFO(DOM) << "        0     1     2     3     4     5     6     7     8   ";
        LOG_INFO(DOM) << "     ╔═════════════════════════════════════════════════════╗";

        for (int i = 0; i < 8; i++) // repeat 8 times
        {
            // row i (tiles, vert walls)
            ClearBuff();    
            sprintf(buff + strlen(buff), "   %d ║", i); // row index
            for (int j = 0; j < 8; j++)
            {
                sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
                sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
            }
            sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
            LOG_INFO(DOM) << buff;

            // row i horiz walls
            ClearBuff();    
            sprintf(buff + strlen(buff), "     ║ ");
            for (int j = 0; j < 8; j++)
            {
                sprintf(buff + strlen(buff), "%c%c%c - ", horizw[hi], horizw[hi], horizw[hi]);
                hi++;
            }
            sprintf(buff + strlen(buff), "%c%c%c ║", horizw[hi], horizw[hi], horizw[hi]);
            hi++;    
            LOG_INFO(DOM) << buff;
        }

        // last row (only tiles and vert walls)
        ClearBuff();    
        sprintf(buff + strlen(buff), "   %d ║", 8); // row index
        for (int j = 0; j < 8; j++)
        {
            sprintf(buff + strlen(buff), "  %c", tiles[ti++]);
            sprintf(buff + strlen(buff), "  %c", vertw[vi++]);
        }
        sprintf(buff + strlen(buff), "  %c  ║", tiles[ti++]);
        LOG_INFO(DOM) << buff;

        LOG_INFO(DOM) << "     ╚═════════════════════════════════════════════════════╝ ";
    }
} // end namespace