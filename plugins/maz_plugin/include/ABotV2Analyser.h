#ifndef Header_qcore_ABotV2Analyser
#define Header_qcore_ABotV2Analyser

#include <deque>
#include <thread>

#include "SolutionNode.h"
#include "MoveInserter.h"



namespace qplugin
{
   class ABotV2Analyser
   {
       public:
         ABotV2Analyser(qcore::PlayerId id);

         void setInputMap(const std::list<qcore::WallState> &currentWalls, std::vector<qcore::PlayerState> playerData);

         Move computeNextMove();

         bool detectCycle(const Move& move);

         void trackMoveHistory(const Move& move);

         MoveInserterSP detectRunStrategy( bool self);
         
         void setOponentMove(const qcore::PlayerAction& lastAction);

         ABBoardCaseNode* minimax(ABBoardCaseNode *node, int depth, bool isMaximizingPlayer, int alpha, int beta);

         void startCountdownTimer(int seconds);

         void stopCountdownTimer();
      
      private:

        qcore::PlayerId m_myPlayerId;

        ABBoardCaseNode m_initialState; // todo: change to smart pointers for dynamic release of the parent nodes

         EverythingInserter m_allMovesGenerator;

         ExtendedHeuristicInserter m_goodMovesGenerator;

         ShortestPathInserter m_shortestPathMovesGenerator;

         std::thread m_countdownTimerThread;

         bool m_cdThreadActive= false;

         bool m_abortComputation = false;

        //int m_crtStrategyRemainingMoves = 0;

        //std::deque<Position> m_prevPosition;

        int m_cycleState = 0;

        int m_childrenCount = 0;
        
#ifdef DUMP_MOVES_LIST
        std::list<Move> m_movesHistory;
#endif
   };
}

#endif // Header_qcore_ABotV2Analyser
