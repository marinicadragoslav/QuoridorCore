#include "MB6_logger.h"
#include "QcoreUtil.h"

namespace qplugin
{
    const char * const DOM = "MB6";

    void MB6_Logger::LogTurnCount(const uint16_t turnCount) const
    {
        LOG_INFO(DOM) << "  >  Turn count = " << turnCount;
    }

    void MB6_Logger::LogMyInfo(const MB6_Board& board) const
    {
        LOG_INFO(DOM) << "  >  Me  (" << (int)(board.mMyID) << ") pos = [" << (int)(board.mPlayerPos[board.mMyID].x) << ", " <<
                (int)(board.mPlayerPos[board.mMyID].y) << "], walls = " << (int)(board.mWallsLeft[board.mMyID]);
    }

    void MB6_Logger::LogOppInfo(const MB6_Board& board) const
    {
        qcore::PlayerId oppID = (board.mMyID ? 0 : 1);
        LOG_INFO(DOM) << "  >  Opp (" << (int)oppID << ") pos = [" << (int)(board.mPlayerPos[oppID].x) << ", " <<
                (int)(board.mPlayerPos[oppID].y) << "], walls = " << (int)(board.mWallsLeft[oppID]);
    }

    void MB6_Logger::LogLastActType(const qcore::ActionType act) const
    {
        LOG_INFO(DOM) << "  >  Last act = " << (act == qcore::ActionType::Move ? "Move" :
                                                    (act == qcore::ActionType::Wall ? "Wall" : "Invalid"));
    }
    
    void MB6_Logger::LogLastActWallInfo(const qcore::Orientation ori, qcore::Position corePos, qcore::Position pluginPos) const
    {
        LOG_INFO(DOM) << "  >     [" << (ori == qcore::Orientation::Horizontal ? "Horizontal" : "Vertical") << "]";
        LOG_INFO(DOM) << "  >     [Core pos]:   " << "[" << (int)corePos.x << ", " << (int)corePos.y << "]";
        LOG_INFO(DOM) << "  >     [Plugin pos]: " << "[" << (int)(pluginPos.x) << ", " << (int)(pluginPos.y) << "]";
    }

    void MB6_Logger::LogMyMinPath(bool pathExists, const MB6_Board& board, uint8_t infinitePath) const
    {
        LOG_INFO(DOM) << "  >  My min path = " << (pathExists ? (int)board.GetMinPath(board.mMyID) : infinitePath);
    }
    
    void MB6_Logger::LogOppMinPath(bool pathExists, const MB6_Board& board, uint8_t infinitePath) const
    {
        qcore::PlayerId oppID = (board.mMyID ? 0 : 1);
        LOG_INFO(DOM) << "  >  Opp min path = " << (pathExists ? (int)board.GetMinPath(oppID) : infinitePath);
    }

    void MB6_Logger::LogInt(int n) const
    {
        LOG_INFO(DOM) << "  >  { " << n << " }";
    }

    #if (DEBUG)
    void MB6_Logger::LogBoard(MB6_Board& board, const qcore::PlayerId myID)
    #else
    void MB6_Logger::LogBoard(const MB6_Board& board, const qcore::PlayerId myID)
    #endif
    {
        char map[qcore::BOARD_MAP_SIZE][qcore::BOARD_MAP_SIZE] = { 0 };

        // add walls to map
        for (int8_t i = 0; i < qcore::BOARD_SIZE; i++)
        {
            for (int8_t j = 0; j < qcore::BOARD_SIZE; j++)
            {
                // convert board coords to map coords
                qcore::Position mapPos = BoardToMapPosition(qcore::Position(i, j));

                // start with an empty tile or a path tile
                #if (DEBUG)
                if ((board.mBoard[i][j] & MB6_Board::TileElements_t::MARKER_MY_PATH) && 
                    (board.mBoard[i][j] & MB6_Board::TileElements_t::MARKER_OPP_PATH)) // tile is on the path of both players
                {
                    map[mapPos.x][mapPos.y] = 'x';
                }
                else if (board.mBoard[i][j] & MB6_Board::TileElements_t::MARKER_MY_PATH) // tile is on my path
                {
                    map[mapPos.x][mapPos.y] = '*';
                }
                else if (board.mBoard[i][j] & MB6_Board::TileElements_t::MARKER_OPP_PATH) // tile is on opponent's path
                {
                    map[mapPos.x][mapPos.y] = '+';
                }
                else // empty tile
                {
                    map[mapPos.x][mapPos.y] = ' ';
                }
                #else
                map[mapPos.x][mapPos.y] = ' ';
                #endif

                // add horizontal wall below if applicable
                if (i < qcore::BOARD_SIZE - 1)
                {
                if (board.mBoard[i][j] & MB6_Board::TileElements_t::WALL_SOUTH_1)
                {
                    map[mapPos.x + 1][mapPos.y] = '-'; // first segment
                }
                else if (board.mBoard[i][j] & MB6_Board::TileElements_t::WALL_SOUTH_2)
                {
                    map[mapPos.x + 1][mapPos.y] = '='; // second segment
                }
                else
                {
                    map[mapPos.x + 1][mapPos.y] = ' ';
                }
                }

                // add vertical wall to the right if applicable
                if (j < qcore::BOARD_SIZE - 1)
                {
                    if (board.mBoard[i][j] & MB6_Board::TileElements_t::WALL_EAST_1)
                    {
                        map[mapPos.x][mapPos.y + 1] = '!'; // first segment
                    }
                    else if (board.mBoard[i][j] & MB6_Board::TileElements_t::WALL_EAST_2)
                    {
                        map[mapPos.x][mapPos.y + 1] = '|'; // second segment
                    }
                    else
                    {
                        map[mapPos.x][mapPos.y + 1] = ' ';
                    }
                }

                // add a dot at the intersection of vertical and horizontal wall lines
                if ((i < qcore::BOARD_SIZE - 1) && (j < qcore::BOARD_SIZE - 1))
                {
                    map[mapPos.x + 1][mapPos.y + 1] = '.';
                }
            }
        }

        // add player positions to the map
        qcore::Position posPlayer0 = BoardToMapPosition(board.mPlayerPos[0]);
        qcore::Position posPlayer1 = BoardToMapPosition(board.mPlayerPos[1]);
        map[posPlayer0.x][posPlayer0.y] = '0';
        map[posPlayer1.x][posPlayer1.y] = '1';

        // log map
        LOG_INFO(DOM) << "  >  ========================== ";
        for (int i = 0; i < qcore::BOARD_MAP_SIZE; i++)
        {
            ClearBuff();
            int bIndex = 0;

            // start of a new line
            mBuff[bIndex++] = ' '; mBuff[bIndex++] = ' '; mBuff[bIndex++] = '>'; mBuff[bIndex++] = ' '; mBuff[bIndex++] = '|';

            for (uint8_t j = 0; j < qcore::BOARD_MAP_SIZE; j++)
            {            
                if ((j % 2) == 0) // check to skip vertical walls and dots
                {
                    if (map[i][j] != '0' && map[i][j] != '1') // check to skip tiles with players
                    {
                        #if (DEBUG)
                        if ((map[i][j] == '+') || (map[i][j] == '*') || (map[i][j] == 'x')) 
                        {
                            // path tiles
                            mBuff[bIndex++] = map[i][j];
                            mBuff[bIndex++] = ' ';
                        }
                        else
                        {
                            // duplicate free tiles and horiz wall tiles
                            mBuff[bIndex++] = map[i][j];
                            mBuff[bIndex++] = map[i][j];
                        }
                        #else
                        // duplicate free tiles and horiz wall tiles
                        mBuff[bIndex++] = map[i][j];
                        mBuff[bIndex++] = map[i][j];
                        #endif
                    }
                    else
                    {
                        // set player tiles to something more meaningful
                        if (map[i][j] - 48 == myID)
                        {
                            mBuff[bIndex++] = 'M';
                            mBuff[bIndex++] = 'E';
                        }
                        else
                        {
                            mBuff[bIndex++] = 'O';
                            mBuff[bIndex++] = 'P';
                        }
                    }
                }
                else
                {
                    // vertical wall tiles and dots
                    mBuff[bIndex++] = map[i][j];
                }
            }
            
            // end the line with a vertical border
            mBuff[bIndex++] = '|';

            LOG_INFO(DOM) << mBuff;
        }
        LOG_INFO(DOM) << "  >  ========================== ";

        #if (DEBUG)
        // remove path info from the board
        for (int8_t i = 0; i < qcore::BOARD_SIZE; i++)
        {
            for (int8_t j = 0; j < qcore::BOARD_SIZE; j++)
            {
                board.mBoard[i][j] &= (~MB6_Board::TileElements_t::MARKER_MY_PATH);
                board.mBoard[i][j] &= (~MB6_Board::TileElements_t::MARKER_OPP_PATH);
            }
        }
        #endif
    }

    qcore::Position MB6_Logger::BoardToMapPosition(const qcore::Position& pos) const
    {
        return pos * 2;
    }

    void MB6_Logger::ClearBuff(void)
    {
        memset(mBuff, 0, sizeof(mBuff));
    }
} // end namespace
