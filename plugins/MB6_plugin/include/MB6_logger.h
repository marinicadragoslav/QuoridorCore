#ifndef H_PLUGIN_MB6_LOGGER
#define H_PLUGIN_MB6_LOGGER

#include "MB6_player.h"
#include "MB6_board.h"

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
            void LogMyMinPath(uint8_t MinPath) const;
            void LogOppMinPath(uint8_t MinPath) const;
            void LogInt(int n) const;

#if (MB6_DEBUG)
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