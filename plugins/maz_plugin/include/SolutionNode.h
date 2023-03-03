#ifndef Header_qcore_SOLUTION_NODE
#define Header_qcore_SOLUTION_NODE

#include "ABBoard.h"
#include "MoveInserter.h"


namespace qplugin
{
	class ABBoardCaseNode;
	using ABBoardCaseNodeSP = std::shared_ptr<ABBoardCaseNode>;

    class ABBoardCaseNode
    {
		public:
		using WallInfo = qcore::WallState;
		
			ABBoardCaseNode(const ABBoard& prevState, PlayerInfo mine, PlayerInfo oponents, ABBoardCaseNode* parent): 
				m_currentState(prevState), m_myState(mine), m_oponentState(oponents), m_parent(parent) {}

			ABBoardCaseNode(const ABBoard& prevState, PlayerInfo mine, PlayerInfo oponents): 
				m_currentState(prevState), m_myState(mine), m_oponentState(oponents) {}

			ABBoardCaseNode(const ABBoardCaseNode& cpy) = delete;

			ABBoardCaseNode() = default;

			void reset();

            void convertQCoreWalls(const std::list<qcore::WallState> &currentWalls);

			void setPlayerInfo(qcore::PlayerState state, bool self);

			inline Position getPlayerPos(bool self) { return self ? m_myState.position : m_oponentState.position;} 

			inline std::vector<WallInfo> getCurrentWalls() { return m_currentState.getWalls(); }

			inline bool wallAllowed(WallInfo wall) {  return m_currentState.wallAllowed(wall.position, wall.orientation == qcore::Orientation::Horizontal);}

			bool addValidWall(WallInfo wall, bool myTurn);

			bool moveTo(Position newPos, bool myTurn);

			bool addNewChild(WallInfo wall, bool myTurn);

			bool addNewChild(bool advanceMe);

			bool addNewChild(Position newPos, bool myTurn);

			inline bool addNewChild(Move move, bool myTurn) { return move.isWallMove ? addNewChild(move.wall, myTurn) : addNewChild(move.pos, myTurn); }

			inline size_t getChildenCount() { return m_children.size();}

			inline void clearChildren() { m_children.clear();}

			inline Move getCurrentMove() { return m_lastMove; }

			inline std::vector<qcore::Position> getShortestPath(bool myPath) 
				{ return  (myPath ?  m_currentState.shortestPath(m_myState.position, qcore::Direction::Up) : m_currentState.shortestPath(m_oponentState.position, qcore::Direction::Down));}

			inline int16_t getScore() {  return m_score; }

			inline bool hasWallsLeft(bool self) { return self ? m_myState.wallsLeft > 0 : m_oponentState.wallsLeft > 0; }

			inline bool hasWon(bool self) {  return self ? m_myState.position.x == 0 : m_oponentState.position.x == 8; }

			inline ABBoardCaseNode* getLastChild() { return m_children.back().get(); }

			void propagateStrategy(const MoveInserterSP& inserter, bool self, int depth);

			void print();

			Move getNextMoveFromRoot();

			bool detectCycleInTrace();

        private:
            ABBoard m_currentState;
			//ABBoardv2 m_currentState;
			int16_t m_score = 0; // ?

			PlayerInfo m_myState = { INVALID_POS, 0};
			PlayerInfo m_oponentState = { INVALID_POS, 0};

			Move m_lastMove;
            
            ABBoardCaseNode* m_parent = nullptr; //lame. Fixme..
            std::list<ABBoardCaseNodeSP> m_children;

#ifdef LOG_MOVES
			public:
			std::string m_name = "R-";
			
#endif	
    };
}

#endif // Header_qcore_SOLUTION_NODE
