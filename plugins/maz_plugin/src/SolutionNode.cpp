#include "SolutionNode.h"




//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc)*5 

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*1 //+ (myWc - opWc)*3 
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc) * 10 * (GlobalData::roundNumber < 12 ? (13 - GlobalData::roundNumber) : 0)
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc) * 7 * (GlobalData::roundNumber < 18 ? (18 - GlobalData::roundNumber) : 0) 
#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc) * 6 * (GlobalData::roundNumber < 19 ? (20 - GlobalData::roundNumber) : 0) 
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100  + (GlobalData::roundNumber < 5 || GlobalData::roundNumber > 16? 0 : (myWc>=opWc ? 100 : 0) )
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc) * 5 * (GlobalData::roundNumber < 22 ? (22 - GlobalData::roundNumber) : 0)

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100  + (GlobalData::roundNumber > 17 ?  0 : (myWc>=opWc ? 50 : 0) )

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc) //Rand: 9 wins 11 loss

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*10 + (GlobalData::roundNumber <= 5 ? (myWc - opWc)*8: (myWc - opWc)*(-8))

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*10 + 5*(myWc - opWc)
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)
 //#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen) *100 + (myWc - opWc) * 2
// #define scoreTraitO(opPLen, myPLen, myWc, opWc) (opPLen - myPLen) * 100

//#define scoreTrait(opPLen, myPLen, myWc, opWc) (myTurn ? scoreTraitM(opPLen, myPLen, myWc, opWc)  : scoreTraitO(opPLen, myPLen, myWc, opWc) )
 
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (myWc - opWc)*(GlobalData::roundNumber > 10 ? 0 : 100 - GlobalData::roundNumber)
//#define scoreTrait(opPLen, myPLen, myWc, opWc) (opPLen - myPLen)*100 + (((myWc - opWc) >= 0 )? 100 : 0)




namespace qplugin
{
	std::string Move::toString(bool qcoreFormat)
	{
		std::string out;
		
		if (isWallMove)
			out =  " W(" + std::to_string((int)wall.position.x+1) +  ":" +  std::to_string((int)wall.position.y+1)  + "-" + (wall.orientation==qcore::Orientation::Horizontal ? "H":"V") + ")";
		else
			out = " Moves to : " + std::to_string((int)pos.x + 1) + ":" + std::to_string((int)pos.y + 1);

		return out;
	}

	Move Move::flip (bool reverse) const
	{
		Move result = *this;

		if (reverse)
		{
			if (result.isWallMove)
			{
				using namespace qcore::literals;
				
				result.wall = result.wall.rotate(2);
				if (result.wall.orientation == qcore::Orientation::Horizontal)
				{
					result.wall.position = result.wall.position - 2_x;
				}
				else
				{
					result.wall.position = result.wall.position - 2_y;
				}
			}
			else
			{
				result.pos = result.pos.rotate(2);
			}
		}
		
		return result;
	}

	bool Move::operator<(const Move& other) const
	{
		int myHash = (isWallMove ? 1:-1)*((wall.position.x*100 + (int)wall.position.y*10+ (wall.orientation==qcore::Orientation::Horizontal ? 1:0))*10 + (pos.x*10+pos.y));
		int otherHash = (other.isWallMove ? 1:-1)*((other.wall.position.x*100 + (int)other.wall.position.y*10+ (other.wall.orientation==qcore::Orientation::Horizontal ? 1:0))*10 + (other.pos.x*10+other.pos.y));

		return myHash < otherHash;
	}

	// bool Move::operator==(const Move& other) const
	// {
	// 	int myHash = (isWallMove ? 1:-1)*((wall.position.x*100 + (int)wall.position.y*10+ (wall.orientation==qcore::Orientation::Horizontal ? 1:0))*10 + (pos.x*10+pos.y));
	// 	int otherHash = (other.isWallMove ? 1:-1)*((other.wall.position.x*100 + (int)other.wall.position.y*10+ (other.wall.orientation==qcore::Orientation::Horizontal ? 1:0))*10 + (other.pos.x*10+other.pos.y));

	// 	return myHash == otherHash;
	// }

   void ABBoardCaseNode::convertQCoreWalls(const std::list<qcore::WallState> &currentWalls)
   {
		using namespace qcore::literals;
		LOG_DEBUG (DOM) <<"ABBoardCaseNode::convertQCoreWalls";

		for(auto &wall : currentWalls)
		{
			if (wall.orientation == qcore::Orientation::Horizontal)
			{
				m_currentState.placeValidWall(wall.position - 1_x, true);
			}
			else
			{
				m_currentState.placeValidWall(wall.position - 1_y, false);
			}
		}

		m_currentState.printState();
   }

   void ABBoardCaseNode::setPlayerInfo(qcore::PlayerState state, bool myTurn) 
   {
		if (myTurn)
		{
			m_myState.position = state.position;
			m_myState.wallsLeft = state.wallsLeft;
			LOG_INFO(DOM) << "My pos : " << (int)m_myState.position.x + 1 << ":" << (int)m_myState.position.y + 1 ;
		}
		else
		{
			m_oponentState.position = state.position;
			m_oponentState.wallsLeft = state.wallsLeft;
			LOG_INFO(DOM) << "Op pos : " << (int)m_oponentState.position.x + 1 << ":" << (int)m_oponentState.position.y + 1 ;
		}
		
		if ((!(m_oponentState.position == INVALID_POS)) && (!(m_myState.position == INVALID_POS)))
		{
			auto mySLen = m_currentState.shortestPathLen(m_myState.position, qcore::Direction::Up);
			auto hisSLen = m_currentState.shortestPathLen(m_oponentState.position, qcore::Direction::Down);

			m_score = scoreTrait(hisSLen, mySLen, m_myState.wallsLeft, m_oponentState.wallsLeft);
			LOG_INFO(DOM) << "Computed score  [" << hisSLen << "-" << mySLen << "]: " << (int)m_score;
		}
	}

	bool ABBoardCaseNode::addValidWall(WallInfo wall, bool myTurn)
	{
		//LOG_DEBUG(DOM) << "placing wall on corner: " << (int)wall.position.x +1<<":"<<(int)wall.position.y+1 << " o: " << (wall.orientation == qcore::Orientation::Horizontal);

		m_currentState.placeValidWall(wall.position, wall.orientation == qcore::Orientation::Horizontal);
		m_lastMove = {wall, INVALID_POS, true};
		auto hisPath =  m_currentState.shortestPathLen(m_oponentState.position, qcore::Direction::Down);
		auto myPath = m_currentState.shortestPathLen(m_myState.position, qcore::Direction::Up);

		if (myPath != -1 && hisPath != -1)
		{
			m_score = scoreTrait(hisPath, myPath, m_myState.wallsLeft, m_oponentState.wallsLeft); 

			if(myTurn)
				m_myState.wallsLeft--;
			else
				m_oponentState.wallsLeft--;
			
			return true;
		}
#ifdef LOG_MOVES
		else
		{
			LOG_INFO (DOM) << "Blocking choice for " << (myPath == -1 ? "me" : "oponent") << " rejecting option";
		}
#endif
		return false;
	}

	bool ABBoardCaseNode::moveTo(Position newPos, bool myTurn)
	{
		if (myTurn == true)
		{
			m_myState.position = newPos;
#ifdef LOG_MOVES
			LOG_INFO(DOM) << "Moving self to :" << (int)m_myState.position.x + 1 << ":" << (int)m_myState.position.y + 1;
#endif
		}
		else
		{
			m_oponentState.position = newPos;
#ifdef LOG_MOVES
			LOG_INFO(DOM) << "Moving OPONENT to :" << (int)m_oponentState.position.x + 1 << ":" << (int)m_oponentState.position.y + 1;
#endif
		}

		m_lastMove = {{INVALID_POS, qcore::Orientation::Horizontal}, newPos, false};
		m_score = scoreTrait(m_currentState.shortestPathLen(m_oponentState.position, qcore::Direction::Down),
				 m_currentState.shortestPathLen(m_myState.position, qcore::Direction::Up), m_myState.wallsLeft, m_oponentState.wallsLeft); 

		return true;
	}

	bool ABBoardCaseNode::addNewChild(WallInfo wall, bool myTurn)
	{
		bool returnVal = false;

		if (m_currentState.wallAllowed(wall.position, wall.orientation == qcore::Orientation::Horizontal))
		{
			ABBoardCaseNodeSP child = std::make_shared<ABBoardCaseNode>(this->m_currentState, this->m_myState, this->m_oponentState, this);

			if (child->addValidWall(wall, myTurn))
			{
				
#ifdef LOG_MOVES
				child->m_name = m_name + std::to_string(m_children.size()) + "-";
				LOG_INFO(DOM) << "Computed (wall) "  << child->m_name  << 
					" W("<< (int)wall.position.x+1 << ":" << (int)wall.position.y+1 <<"-" << (wall.orientation==qcore::Orientation::Horizontal? "H":"V") << "  child score: " << (int)child->m_score;
#endif
				m_children.push_back(child);

				returnVal = true;
			}
		}

		return returnVal;
	}

	bool ABBoardCaseNode::addNewChild(Position newPos, bool myTurn)
	{
		bool retVal = false;

		if (newPos == INVALID_POS)
			return addNewChild(myTurn); // special case for invalid_pos : follow shortest path
#ifdef LOG_MOVES
		LOG_INFO(DOM) << "ABBoardCaseNode::addNewChild(Position newPos, bool myTurn) : " << (int)newPos.x + 1 << ":" <<(int)newPos.y + 1;
#endif

		
		Position oldPos = myTurn ? m_myState.position : m_oponentState.position;
		Position oponentPos = myTurn ? m_oponentState.position : m_myState.position;

		if (m_currentState.moveAllowed(oldPos, newPos) )
		{
			if (!(newPos == oponentPos))
			{
				ABBoardCaseNodeSP child = std::make_shared<ABBoardCaseNode>(this->m_currentState, this->m_myState, this->m_oponentState, this);

				if (child->moveTo(newPos, myTurn))
				{
#ifdef LOG_MOVES
					child->m_name = m_name + std::to_string(m_children.size()) + "-";
					LOG_INFO(DOM) << "Computed (move) " << child->m_name  << " Moves to : " <<(int)newPos.x + 1 << ":" <<(int)newPos.y + 1 << " child score: " << (int)child->m_score;
#endif
					m_children.push_back(child);
					retVal = true;
				}
			}
			else
			{
#ifdef LOG_MOVES
				LOG_INFO(DOM) <<  "Player overlap ! Jumping over oponent";
#endif
				std::vector<qcore::Position> shortestPath = m_currentState.shortestPath(newPos, myTurn ? qcore::Direction::Up:qcore::Direction::Down);
				
				if (shortestPath.empty()) //oponent is standing on my finish point
				{
#ifdef LOG_MOVES
					LOG_INFO(DOM) <<  "Oponent on finish point. Doing side move";
#endif

					if (((newPos.y +1) <  qcore::BOARD_SIZE) && m_currentState.moveAllowed(newPos, {newPos.x, newPos.y + 1}))
						newPos =  {newPos.x, newPos.y + 1};
					else if (((newPos.y) >  0) && m_currentState.moveAllowed(newPos, {newPos.x, newPos.y - 1}))
						newPos =  {newPos.x, newPos.y - 1};
#ifdef LOG_MOVES
					LOG_INFO(DOM) <<  "NewPos: " <<(int)newPos.x +1 << ":" << (int)newPos.y +1;
#endif
				}
				else
				{
					newPos = *std::prev(shortestPath.end());
				}
#ifdef LOG_MOVES
				LOG_INFO(DOM) <<  "NewPos: " <<(int)newPos.x +1 << ":" << (int)newPos.y +1;
#endif


				ABBoardCaseNodeSP child = std::make_shared<ABBoardCaseNode>(this->m_currentState, this->m_myState, this->m_oponentState, this);

				if (child->moveTo(newPos, myTurn) &&  (!(newPos == oldPos)))
				{
#ifdef LOG_MOVES
					child->m_name = m_name + std::to_string(m_children.size()) + "-";
					LOG_INFO(DOM) << "Computed (move-jmp) " << child->m_name  << " Moves to : " <<(int)newPos.x + 1 << ":" <<(int)newPos.y + 1 << " child score: " << (int)child->m_score;
#endif
					m_children.push_back(child);
					retVal = true;
				}
				else
				{
					retVal = false;
				}
			}

			
		}
		#ifdef LOG_MOVES
		else
			LOG_INFO(DOM) << "Move to new pos, rejected." << "Old: " << (int)oldPos.x +1 << ":"<<(int)oldPos.y +1  << " - new: " << (int)newPos.x+1 << ":" <<(int)newPos.y+1 <<
				" reason: allowed: " << m_currentState.moveAllowed(oldPos, newPos);
		#endif

		return retVal;
	}


	bool ABBoardCaseNode::addNewChild(bool myTurn) // advance using shortest path
	{

		ABBoardCaseNodeSP child = std::make_shared<ABBoardCaseNode>(this->m_currentState, this->m_myState, this->m_oponentState);


		std::vector<qcore::Position> shortestPath = getShortestPath(myTurn);
		auto newPos = shortestPath.back();

		if (newPos == m_myState.position || newPos == m_oponentState.position)
		{
			LOG_INFO(DOM) << "Computing jump over oponent";

			newPos = *std::prev(std::prev(shortestPath.end()));

			if (newPos == m_myState.position || newPos == m_oponentState.position) //oponent is standing on my finish point
			{
				if (((newPos.y +1) <  qcore::BOARD_SIZE) && m_currentState.moveAllowed(newPos, {newPos.x, newPos.y + 1}))
					newPos =  {newPos.x, newPos.y + 1};
				else if (((newPos.y) >  0) && m_currentState.moveAllowed(newPos, {newPos.x, newPos.y - 1}))
					newPos =  {newPos.x, newPos.y - 1};
			}
		}

		if (myTurn == true)
		{
			child-> m_myState.position = newPos;
		}
		else
		{
			child->m_oponentState.position = newPos;
		}

		child->m_score = scoreTrait(child->m_currentState.shortestPathLen(m_oponentState.position, qcore::Direction::Down),
				 child->m_currentState.shortestPathLen(m_myState.position, qcore::Direction::Up), child->m_myState.wallsLeft, child->m_oponentState.wallsLeft); 
		child->m_lastMove =  {{INVALID_POS, qcore::Orientation::Horizontal}, newPos, false};
		child->m_parent = this;

		// if (m_oponentState.wallsLeft == 0) // take advantage of the situation. //todo: checkMe !!!
		// 	child->m_score = child->m_score - 1;

		
#ifdef LOG_MOVES
		child->m_name = m_name + std::to_string(m_children.size()) + "-";
		LOG_INFO(DOM) << "Computed (fwd) child " << child->m_name  <<" score: " << (int)child->m_score << "nwPos: " << (int)newPos.x + 1 << ":" << (int)newPos.y +1;
#endif
		m_children.push_back(child);

		return true;
	}

	void ABBoardCaseNode::propagateStrategy(const MoveInserterSP& inserter, bool self, int depth)
	{
		if (depth == 0)
			return;

		for (auto& mvOption : inserter->getMoves(this, self))
        {
             addNewChild(mvOption, self);
        }

		for(auto &node : m_children)
		{
			node->propagateStrategy(inserter, !self, depth - 1);
		}
	}

	

	void ABBoardCaseNode::print()
	{
#ifdef LOG_MOVES
		LOG_INFO(DOM) << "Name: " << m_name;
#endif
		LOG_INFO(DOM) << "My pos : " << (int)m_myState.position.x + 1 << ":" << (int)m_myState.position.y + 1 << " Walls: " << (int)m_myState.wallsLeft ;
		LOG_INFO(DOM) << "Op pos : " << (int)m_oponentState.position.x + 1 << ":" << (int)m_oponentState.position.y + 1 << " Walls: " << (int)m_oponentState.wallsLeft ;
		
		if (m_lastMove.isWallMove)
		{
			LOG_INFO(DOM) << " Wall to: " << (int)m_lastMove.wall.position.x+1 << ":" << (int)m_lastMove.wall.position.y +1<< " H: " << (m_lastMove.wall.orientation == qcore::Orientation::Horizontal);
		}
		else
		{
			LOG_INFO(DOM) << " Move to: " << (int)m_lastMove.pos.x+1 << ":" << (int)m_lastMove.pos.y+1;
		}
		LOG_INFO(DOM) << "Score: "<< (int)m_score;

		m_currentState.printState();
	}

	bool ABBoardCaseNode::detectCycleInTrace()
	{
		if (this->m_parent &&
			this->m_parent->m_parent &&
			this->m_parent->m_parent->m_parent &&
			this->m_parent->m_parent->m_parent->m_parent)
		{
			//Position L4 = this->m_myState.position;
			Position L3 = this->m_parent->m_myState.position;
			//Position L2 = this->m_parent->m_parent->m_myState.position;
			Position L1 = this->m_parent->m_parent->m_parent->m_myState.position;
			Position L0 = this->m_parent->m_parent->m_parent->m_parent->m_myState.position;

			bool result = !(L0 == L1) && L0 ==L3;

#ifdef LOG_MOVES
			LOG_INFO(DOM) << "Cycle check: L0 : ["<<(int)L0.x +1 <<":" << (int)L0.y  +1 << "] \n" <<
			"Cycle check: L1 : ["<<(int)L1.x +1 <<":" << (int)L1.y  +1 << "] \n" <<
			//"Cycle check: L2 : ["<<(int)L2.x +1 <<":" << (int)L1.y  +1 << "] \n" <<
			"Cycle check: L3 : ["<<(int)L3.x +1 <<":" << (int)L3.y  +1 << "] \n";

			if (result)
			{
				LOG_INFO(DOM) << "Cycle check: true";
			}
#endif

			return result;
		}
		else
		{
			LOG_INFO(DOM) << "Cycle detect fail ! Pwla calului! ";
		}

		return false;
	}

	Move ABBoardCaseNode::getNextMoveFromRoot() // todo: this is bad. Fixme !
	{
		ABBoardCaseNode* node = this;

		LOG_INFO(DOM) <<"\nTracing solution tree (reversed): ";
		int lineNo = 1;
		LOG_INFO(DOM) << lineNo++ << ":";
		node->print();

		if (node->m_parent)
		{
			while( node->m_parent->m_parent != nullptr)
			{
				node = node->m_parent;
				LOG_INFO(DOM) << lineNo++ << ":";
				node->print();
			}
		}
		else
		{
			LOG_ERROR(DOM) << "FATAL ERROR. This is soo bad ! :)";
		}
		
		return node->m_lastMove;
	}

	void ABBoardCaseNode::reset()
	{
		LOG_INFO (DOM) <<"ABBoardCaseNode::reset()";

		m_currentState = ABBoard();
		m_myState.position = INVALID_POS;
		m_oponentState.position = INVALID_POS;
		m_score = 0; 
		//m_lastMove;
        m_parent = nullptr;
        m_children.clear();

	}
}