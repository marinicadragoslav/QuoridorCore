#ifndef Header_qcore_MOVE_INSERTER
#define Header_qcore_MOVE_INSERTER

#include <set>
#include <memory>

namespace qplugin
{
    struct Move;
    class ABBoardCaseNode;

    class MoveInserter
    {
        public:
        //MoveInserter(ABBoardCaseNodeSP crtBoardState, bool self) : m_startBoardPtr(crtBoardState), m_self(self) {}
        virtual ~MoveInserter() = default;

        // inline void updateBoard(ABBoardCaseNodeSP crtBoardState, bool self) { m_startBoardPtr = crtBoardState; m_self = self;}
        
        virtual std::set<Move> getMoves(ABBoardCaseNode* crtBoardState, bool self) = 0;
        virtual int minRoundCount() = 0;
        virtual int recursionDepth() = 0;
    };

    using MoveInserterSP = std::shared_ptr<MoveInserter> ;

    class EverythingInserter : public MoveInserter
    {
        public:
        //EverythingInserter(ABBoardCaseNodeSP crtBoardState, bool self) : MoveInserter(crtBoardState, self) {}
        std::set<Move> getMoves(ABBoardCaseNode* crtBoardState, bool self) override;
        inline int minRoundCount() override {  return 1;}
        inline int recursionDepth() override {  return 3;}
    };

    class ExtendedHeuristicInserter : public MoveInserter
    {
        public:
        //EverythingInserter(ABBoardCaseNodeSP crtBoardState, bool self) : MoveInserter(crtBoardState, self) {}
        std::set<Move> getMoves(ABBoardCaseNode* crtBoardState, bool self) override;
        inline int minRoundCount() override {  return 1;}
        inline int recursionDepth() override {  return 2;}
    };

    class ShortestPathInserter : public MoveInserter
    {
        public:
        std::set<Move> getMoves(ABBoardCaseNode* crtBoardState, bool self) override;
        inline int minRoundCount() override {  return 1;}
        inline int recursionDepth() override {  return 2;}
    };

    class SchilerInserter : public MoveInserter
    {
        public:
        //SchilerInserter(ABBoardCaseNodeSP crtBoardState, bool self) : MoveInserter(crtBoardState, self) {}
        std::set<Move> getMoves(ABBoardCaseNode* crtBoardState, bool self) override;
        inline int minRoundCount() override { return 3;}
        inline int recursionDepth() override { return 1;}

        private:
            int m_callCount= 0;
    };
}
#endif // Header_qcore_MOVE_INSERTER
