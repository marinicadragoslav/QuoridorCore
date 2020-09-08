/*
 * TurnGenerator.cpp
 *
 *  Created on: May 3, 2020
 *      Author: andrei
 */

#include "TurnGenerator.h"
#include <iostream>
#include <algorithm>
#include <memory>

#include <thread>
#include <future>
#include <chrono>

#include "QcoreUtil.h"

namespace TermAi
{

int TurnGenerator::m_currentDepth = 1;

int ObjsConstructed = 0;
auto started = std::chrono::high_resolution_clock::now();

using namespace std;

TurnGenerator::TurnGenerator(BoardPtr b, uint8_t player, uint8_t myWallsLeft, uint8_t opWallsLeft ) //: m_initialBoard(b)
{
	m_initialBoard = b;
	m_player = player;
	m_self = true;
	m_indexInParent = -1;
	m_currentDepth = 1;
	m_parent = nullptr;
	m_name = "R-";

	m_ownWallsLeft = myWallsLeft;
	m_opWallsLeft = opWallsLeft;

	LOG_DEBUG(DOM)<<"received walls left: Own:" << (int)m_ownWallsLeft << " op: " <<(int)m_opWallsLeft;

	ObjsConstructed = 1;
	started = std::chrono::high_resolution_clock::now();

	//Board::targetOnlyPathDiffs = false;
}
TurnGenerator::~TurnGenerator()
{
	m_parent = nullptr;
	m_moveList.clear();
}

TurnGenerator::TurnGenerator( TurnGenerator* parent, BoardPtr b, uint8_t childIdx)
{
	m_initialBoard = b;
	m_indexInParent = childIdx;
	m_self = not parent->m_self;
	m_parent = parent;
	m_ownWallsLeft = parent->m_ownWallsLeft;
	m_opWallsLeft = parent->m_opWallsLeft;

	if (m_initialBoard->m_lastMove.moveType > ADVANCE )
	{
		if (not m_self) // object constructed through a m_self = true parent
		{
			m_ownWallsLeft--;
			LOG_DEBUG(DOM)<<"m_ownWallsLeft : " << (int)m_ownWallsLeft;
		}
		else
		{
			m_opWallsLeft--;
			LOG_DEBUG(DOM)<<"m_opWallsLeft : " << (int)m_opWallsLeft;
		}
	}

	m_player = (parent->m_player == Player_1 ? Player_2: Player_1);

	m_name = parent->m_name + std::to_string((int)childIdx) + "-";

	LOG_DEBUG(DOM)<<"\nnew TurnGenerator (childIdx: "<<(int)childIdx<<"): ";

	ObjsConstructed++;
}

std::list<Move> TurnGenerator::get_moves()
{
	TurnGenerator* p = get_best_gen();

	std::list<Move> moveList;
	while (p->m_parent)
	{

		LOG_DEBUG(DOM) << "Node: "<< p->m_name << " Move type: " <<(int)p->m_initialBoard->m_lastMove.moveType <<" Score: " <<  (int)p->m_initialBoard->m_lastMove.score << "At: "<<(int)p->m_initialBoard->m_lastMove.m_location.first << ":"<<(int)p->m_initialBoard->m_lastMove.m_location.second;
		moveList.push_back( p->m_initialBoard->m_lastMove );

		p = p->m_parent;
	}

	moveList.reverse();

	LOG_DEBUG(DOM)<<"\n\nSelected move list: ";
	for(auto mv : moveList)
	{
		LOG_DEBUG(DOM) <<" Move type: " <<(int)mv.moveType <<" Score: " <<  (int)mv.score << "At: "<<(int)mv.m_location.first << ":"<<(int)mv.m_location.second;
	}


	return moveList;
}

bool TurnGenerator::add_move( BoardPtr generatedMove )
{
	if ( generatedMove->m_isValid )
	{
		//avoid duplicates (e.g. multi-prevent)
		uint16_t key = (generatedMove->m_lastMove.moveType*100) + (generatedMove->m_lastMove.m_location.first * 10) + generatedMove->m_lastMove.m_location.second;

		if (std::binary_search(m_createdNodes.begin(), m_createdNodes.end(), key) == false)
		{
			m_createdNodes.emplace(key);

			m_moveList.push_back( TurnGeneratorPtr( new TurnGenerator(this, generatedMove, m_moveList.size() ) ) );

			return true;
		}
	}
	return false;
}

void TurnGenerator::compute()
{
	LOG_DEBUG(DOM) << "TurnGenerator compute enters. Player: " <<(int)m_player << ". Self: " << m_self ;



	LOG_DEBUG(DOM)<<"\n\n-----------------------------------------------------\nCompute for: "<< m_name;

	LOG_DEBUG(DOM)<<"\nInitial :";
	m_initialBoard->printCost();

	uint8_t oponent = (m_player == Player_1 ? Player_2 : Player_1);

	//1. Add advance option
	add_move( m_initialBoard->c_advance(m_player) );

	Move bestMove = (*m_moveList.begin())->m_initialBoard->m_lastMove;

	//2. disrupt opponent shortest path
	if ( (m_self and m_ownWallsLeft > 0) or (not m_self and m_opWallsLeft > 0) )
	{
		std::pair<uint8_t, uint8_t> frontCell;
		CellSides side;
		BoardPtr generatedMove;
		uint8_t index = 0;

		while ( m_initialBoard->get_diff_path_cell(oponent, frontCell, side, index++)  ) // ? heuristic: only diff - for efficiency vs all since some blocks prove a greater advantage
		{
			switch(side)
			{
			case LEFT:
				add_move( m_initialBoard->c_place_wall(frontCell.first, frontCell.second - 1, false) );
				add_move( m_initialBoard->c_place_wall(frontCell.first - 1, frontCell.second - 1, false) );
				break;
			case RIGHT:
				add_move( m_initialBoard->c_place_wall(frontCell.first, frontCell.second, false) );
				add_move( m_initialBoard->c_place_wall(frontCell.first - 1, frontCell.second, false) );

				break;
			case UP:
				add_move( m_initialBoard->c_place_wall(frontCell.first - 1, frontCell.second, true) );
				add_move( m_initialBoard->c_place_wall(frontCell.first - 1, frontCell.second - 1, true) );
				break;
			case DOWN:
				add_move( m_initialBoard->c_place_wall(frontCell.first, frontCell.second, true) );
				add_move( m_initialBoard->c_place_wall(frontCell.first, frontCell.second - 1 , true) );
				break;
			default:
				break;

			}
		}

		if (m_self)
		{
			bestMove = (*max_element(m_moveList.begin(), m_moveList.end(),
				[](const TurnGeneratorPtr& lhs, const TurnGeneratorPtr& rhs) { return lhs->m_initialBoard->m_lastMove.score < rhs->m_initialBoard->m_lastMove.score; })
					)->m_initialBoard->m_lastMove;
		}
		else
		{
			bestMove = (*min_element(m_moveList.begin(), m_moveList.end(),
							[](const TurnGeneratorPtr& lhs, const TurnGeneratorPtr& rhs) { return lhs->m_initialBoard->m_lastMove.score < rhs->m_initialBoard->m_lastMove.score; })
								)->m_initialBoard->m_lastMove;
		}

		LOG_DEBUG(DOM)<< " Best move: " <<(int)bestMove.moveType <<" Score: " <<  (int)bestMove.score << "At: "<<(int)bestMove.m_location.first << ":"<<(int)bestMove.m_location.second;

		LOG_DEBUG(DOM)<<"Removing trivial elements:"; // for both ?? Not sure if a good idea (this will destroy good planning)

		if (not m_self)
		{
			m_moveList.remove_if([x=bestMove.score](TurnGeneratorPtr n){ return n->m_initialBoard->m_lastMove.score >x; });
		}
		else
		{
			m_moveList.remove_if([x=bestMove.score](TurnGeneratorPtr n){ return n->m_initialBoard->m_lastMove.score < x; });
		}


		if ( ( not m_self and m_initialBoard->m_lastMove.score > (bestMove.score + 1) ) or
					( m_self and m_initialBoard->m_lastMove.score < (bestMove.score - 1) )
				)
		{
			//significant drop !
			if (m_parent)
			{
				LOG_DEBUG(DOM)<<"prevent_move: myScore: " << (int)m_initialBoard->m_lastMove.score << " VS " << (int)bestMove.score ;

				m_parent->handle_prevent_move(bestMove);
			}
		}

	}
	else
	{
		LOG_DEBUG(DOM)<<"No more walls left :(. Self: "<<m_self;
	}


	// Fix child indexes(since some were removed some were added by prevent)
	restore_child_indexes();

	LOG_DEBUG(DOM) << "TurnGenerator compute done. Player: " <<(int)m_player << ". Self: " << m_self << "Index: " << m_indexInParent << " Moving on/";

	LOG_DEBUG(DOM)<<"\n\n-----------------------------------------------------\nEnd Compute for: "<< m_name<< " Children: " << m_moveList.size();

	LOG_DEBUG(DOM)<<"Children score : ";
	for(auto move : m_moveList)
	{
		LOG_DEBUG(DOM)<<(int)move->m_initialBoard->m_lastMove.score<<"-";
	}
	LOG_DEBUG(DOM);

	LOG_DEBUG(DOM)<<"\n\n\n";

	m_childComputed.assign( m_moveList.size(), false);

	if ( m_indexInParent != -1 && m_parent )
	{
		m_parent->report_finished( m_indexInParent );
	}
	else
	{
		// This is root - start processing lvl 1.
		process_depth_level(1,m_moveList);

		auto done = std::chrono::high_resolution_clock::now();

		if (LOG_LEVEL  <= qcore::util::Log::Debug)
		{
			print_gen_tree(1, this);
		}

		LOG_INFO(DOM)<<"COMPLETE ! Total objs: " << ObjsConstructed << " Time ms: " << std::chrono::duration_cast<std::chrono::milliseconds>(done-started).count() ;

		LOG_DEBUG(DOM)<<"\nInitial :";
		m_initialBoard->printCost();
	}
}


void TurnGenerator::handle_prevent_move(Move &dbg)
{
	// Sync !

	if ( (m_self and m_ownWallsLeft > 0) or (not m_self and m_opWallsLeft > 0) )
	{
		LOG_DEBUG(DOM)<<" UH OH ! - we should prevent this move: " <<(int)dbg.moveType <<" Score: " <<  (int)dbg.score << "At: "<<(int)dbg.m_location.first << ":"<<(int)dbg.m_location.second;

		if (dbg.moveType == PLACE_WALL_H)
		{
			if (add_move( m_initialBoard->c_place_wall(dbg.m_location.first, dbg.m_location.second, false) ))
			{
				m_childComputed.push_back(false);

				add_pending_processing( *std::prev(m_moveList.end(),1) );
			}
		}
		else if (dbg.moveType == PLACE_WALL_V)
		{
			if (add_move( m_initialBoard->c_place_wall(dbg.m_location.first, dbg.m_location.second, true) ))
			{
				m_childComputed.push_back(false);

				add_pending_processing( *std::prev(m_moveList.end(),1) );
			}
		}
	}
	else
	{
		LOG_DEBUG(DOM)<<"No walls left to prevent this :() ";
	}
}

// Key function -and it might have some bugs - since tree is generated ok - but selection done by this function is not always great.
// Idea: parse solution tree - self will maximize, oponent will minimize
TurnGenerator* TurnGenerator::get_best_gen()
{
	std::string myName = m_name + ":";

	if (m_moveList.size() == 0) // leaf
	{
		return this;
	}
	else
	{
		TurnGenerator* best = nullptr;

		for (auto it : m_moveList)
		{
			TurnGenerator* cmp = it->get_best_gen();

			if ( (not best) or (
					( m_self and (best->m_initialBoard->m_lastMove.score < cmp->m_initialBoard->m_lastMove.score)) or
					((not m_self) and (best->m_initialBoard->m_lastMove.score > cmp->m_initialBoard->m_lastMove.score))  )
				)
			{
				best = cmp;
			}
		}

		return best;
	}
}

void TurnGenerator::restore_child_indexes()
{
	int i=0;

	for(auto move : m_moveList)
	{
		move->m_name = m_name + std::to_string(i) + "-";
		move->m_indexInParent = i++;
	}
}

void TurnGenerator::report_finished( int16_t childIndex )
{
	//todo:CAUTION on sync !! - all members

	LOG_DEBUG(DOM)<<m_name<<" Received result from child: " <<(int)childIndex;// << " Best move: " <<(int)bestMove.moveType <<" Score: " <<  (int)bestMove.score << "At: "<<(int)bestMove.m_location.first << ":"<<(int)bestMove.m_location.second;

	m_childComputed[childIndex] = true;

	if ( std::find(m_childComputed.begin(), m_childComputed.end(), false) == m_childComputed.end() )
	{
		LOG_DEBUG(DOM)<<"All results received ("<< m_childComputed.size() <<") ";

		if ( m_indexInParent != -1 and m_parent )
		{
			if ( m_moveList.size() > 0 )
			{ // not a leaf
				m_parent->report_finished( m_indexInParent);
			}
		}
		else // reached top
		{
			m_currentDepth++;
			LOG_DEBUG(DOM)<<"Reached top. Increasing depth. \n\n\n\n\n\n";

//			if (m_currentDepth > (MAX_DEPTH/2))
//			{
//				Board::targetOnlyPathDiffs = true;
//			}

			if (m_currentDepth < MAX_DEPTH)
			{
				m_childComputed.assign( m_moveList.size(), false );
				process_depth_level(1,m_moveList);
			}
			else
			{
				LOG_DEBUG(DOM)<<"FINISH ! ";
			}
		}
	}
}

void TurnGenerator::add_pending_processing( TurnGeneratorPtr move )
{
	// todo: add to thread pool for parallel processing (sync on parent reports and parent::prevent move)

	move->compute();

}

void TurnGenerator::process_depth_level(uint8_t recLvl, std::list<TurnGeneratorPtr> moveList)
{
	if (m_moveList.size() > 0)
	{
		if (recLvl == m_currentDepth)
		{
			for (auto move : moveList)
			{
				add_pending_processing(move);
			}
		}
		else
		{
			for (auto move : moveList)
			{
				move->m_childComputed.assign(move->m_moveList.size(), false);
				process_depth_level(recLvl+1, move->m_moveList);
			}
		}
	}
}

void TurnGenerator::print_gen_tree(int recLvl, TurnGenerator *obj)
{
	std::string tab;
	for(int i=1;i<recLvl;i++)
		tab +="   ";

	Move mv = obj->m_initialBoard->m_lastMove;

	LOG_DEBUG(DOM)<<tab<<"Name: "<<obj->m_name << " Own move: " <<(int)mv.moveType <<" OwnScore: " <<  (int)mv.score << " At: "<<(int)mv.m_location.first << ":"<<(int)mv.m_location.second;

	for(auto move : obj->m_moveList)
	{
		print_gen_tree( recLvl+1, &(*move) );
	}
}

}
