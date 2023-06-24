#ifndef H_PLUGIN_MB6_LOGGER
#define H_PLUGIN_MB6_LOGGER

#include "MB6_player.h"

namespace qplugin
{
    class MB6_Logger
    {
        public:
            void LogTurnCount(const uint16_t turnCount) const;
            void LogMyInfo(const MB6_Board& board) const;
            void LogOppInfo(const MB6_Board& board) const;
            void LogLastActType(const qcore::ActionType act) const;
            void LogLastActWallInfo(const qcore::Orientation ori, qcore::Position corePos, qcore::Position pluginPos) const;
            void LogMyMinPath(bool pathExists, const MB6_Board& board, uint8_t infinitePath) const;
            void LogOppMinPath(bool pathExists, const MB6_Board& board, uint8_t infinitePath) const;
            void LogInt(int n) const;

            #if (DEBUG)
            void LogBoard(MB6_Board& board, const qcore::PlayerId myID);
            #else
            void LogBoard(const MB6_Board& board, const qcore::PlayerId myID);
            #endif

        private:
            qcore::Position BoardToMapPosition(const qcore::Position& pos) const;
            void ClearBuff(void);

            char mBuff[LOGGED_LINE_MAX_LEN] = { 0 };
    };
} // end namespace

#endif // H_PLUGIN_MB6_LOGGER