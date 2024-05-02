#ifndef H_PLUGIN_MB6_LOGGER
#define H_PLUGIN_MB6_LOGGER

#include "PlayerAction.h"
#include "MB6_board.h"
#include <vector>

#define LINE_MAX_LEN    (120)

namespace qplugin
{
    class MB6_Logger
    {
        public:
            void LogTurnCount(const uint16_t turnCount) const;
            void LogMyInfo(const MB6_Board& board) const;
            void LogOppInfo(const MB6_Board& board) const;
            void LogLastActType(const qcore::ActionType& act) const;
            void LogLastActWallInfo(const qcore::Orientation& ori, 
                                    const qcore::Position& corePos, 
                                    const qcore::Position& pluginPos) const;
            void LogMyMinPath(const uint8_t MinPath) const;
            void LogOppMinPath(const uint8_t MinPath) const;
            void LogBestAction(const Action_t act) const;

#if (DEBUG_BOARD)
            void LogBoard(MB6_Board& board);
#else
            void LogBoard(const MB6_Board& board);
#endif

        private:
            qcore::Position BoardToMapPosition(const qcore::Position& pos) const;
            void ClearBuff(void);

            char mBuff[LINE_MAX_LEN] = { 0 };
    };
} // end namespace

#endif // H_PLUGIN_MB6_LOGGER