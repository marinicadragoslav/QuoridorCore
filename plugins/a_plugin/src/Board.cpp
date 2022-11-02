/*
 * Board.cpp
 *
 *  Created on: Feb 18, 2020
 *      Author: andrei
 */

#include <iostream>
#include <list>
#include <iomanip>
#include <valarray>

#include "Board.h"

#include "QcoreUtil.h"


using namespace std;

namespace TermAi
{

	Move::Move()
	{
		moveType = INVALID;
		score = 0;
	}

	void Move::set_move_type(MoveType type, Coord pos)
	{
		moveType = type;
		m_location = pos;
	}

	BoardPtr Board::m_invalidBoard  = std::make_shared<Board>();

	bool Board::targetOnlyPathDiffs = ONLY_PATH_DIFFS;


	Board::Board() : m_isValid(true), m_gameOver(false)
	{
		update_links();
	}

	Board::Board(
			std::vector<qcore::PlayerState> players,
			std::list<qcore::WallState> walls) : Board()
	{
		//todo: init once
		m_invalidBoard->m_isValid = false;

		for (auto wall : walls)
		{
			if (wall.orientation == qcore::Orientation::Vertical)
			{
				place_wall(wall.position.x + 1, wall.position.y, false);
			}
			else
			{
				place_wall(wall.position.x, wall.position.y+1, true);
			}
		}
		for (uint8_t i=0;i<players.size();i++)
		{
			m_player_cell[i] = std::make_pair(players[i].position.x+1, players[i].position.y+1);
		}

		refresh_info();
	}


	Board::Board(const Board &b2)
	{
		std::copy( b2.m_board, b2.m_board + (BOARD_SIZE * BOARD_SIZE), m_board );
		std::copy(b2.m_player_cell, b2.m_player_cell + Player_last, m_player_cell );

		m_gameOver = b2.m_gameOver;
		m_isValid = b2.m_isValid;

	}

	void Board::refresh_info()
	{
		LOG_TRACE(DOM)<<"Refreshing paths";

		for (uint8_t i=0;i<Player_last;i++)
		{
			m_shortest_path[i] = shortest_path(i);

			if (m_shortest_path[i].size() == 0)
			{
				LOG_DEBUG(DOM) << "Invalid board. Player "<<(int)i<<"is locked out";
				m_isValid = false;
			}
		}

		m_lastMove.score = (m_shortest_path[Player_1].size() - m_shortest_path[Player_2].size());
	}

	bool Board::is_player_finish_pt(Cell *c, uint8_t player)
	{
		if (player == Player_1)
			return ((c-m_board) < BOARD_SIZE);
		else
			return ((c-m_board) >= (BOARD_SIZE * (BOARD_SIZE - 1)));
	}

	// Set board boundaries
	void Board::update_links()
	{
		// todo: default Cell to LEFT|RIGHT|UP|DOWN and set boundaries to false - for less assignments

		for (int i=0;i<BOARD_SIZE * BOARD_SIZE;i++)
		{
			if ((i%BOARD_SIZE)>0)
				m_board[i].left(true);

			if ((i%BOARD_SIZE)<(BOARD_SIZE-1))
				m_board[i].right(true);

			if (i>(BOARD_SIZE-1))
				m_board[i].up(true);

			if (i<(BOARD_SIZE*(BOARD_SIZE-1)))
				m_board[i].down(true);
		}
	}


	Cell* Board::getCell(uint8_t row, uint8_t col)
	{
		if (row > 0 and col > 0 and row <= BOARD_SIZE and col <= BOARD_SIZE )
			return &m_board[(row-1) * BOARD_SIZE + (col-1)];
		else
		{
			LOG_WARN(DOM)<< "ERROR - invalid cell requested (" <<(int)row<<", " <<(int)col << ")";
			return nullptr;
		}
	}

	// Internal function. Should only be called after "can_place_wall"
	void Board::place_wall(uint8_t row, uint8_t col, bool horizontal)
	{
		LOG_DEBUG(DOM) << "placing wall on corner: " << (int)row<<":"<<(int)col << "o: " << horizontal;

		if (horizontal)
		{
			getCell(row, col)->down()->up(false); //corner->right()->down()->up(false);
			getCell(row, col)->down(false);
			getCell(row, col+1)->down()->up(false);
			getCell(row, col+1)->down(false);
		}
		else
		{
			getCell(row, col)->right()->left(false);
			getCell(row, col)->right(false);
			getCell(row+1, col)->right()->left(false);
			getCell(row+1, col)->right(false);
		}
	}

	bool Board::can_place_wall(uint8_t row, uint8_t col, bool horizontal)
	{
		if (row > 0 and col > 0 and row <= BOARD_SIZE and col <= BOARD_SIZE )
		{
			Cell *corner = getCell(row, col);

			if (horizontal and ( (corner->down() and (
							(corner->right() and corner->right()->down()) or
							(corner->down()->right() and corner->down()->right()->up()))  ) ))
			{
				return true;
			}
			else if ( (not horizontal) and (corner->right() and (
									(corner->down() and corner->down()->right()) or
									(corner->right()->down() and corner->right()->down()->left())
									)) )
			{
				return true;
			}
		}

		return false;
	}

	BoardPtr Board::c_place_wall(uint8_t row, uint8_t col, bool horizontal)
	{
		if (can_place_wall(row, col, horizontal))
		{
			BoardPtr result(new Board(*this));
			result->place_wall(row, col, horizontal);

			if (result->m_isValid)
			{
				result->refresh_info();

				result->m_lastMove.set_move_type(
						horizontal ? PLACE_WALL_H: PLACE_WALL_V, {row,col} );

				LOG_DEBUG(DOM)<<"c_place_wall raw score: " << (int)result->m_lastMove.score;
			}

			return result;
		}
		else
		{
			LOG_DEBUG(DOM)<<"invalid move: wall ("<< (int)row << ":"<< (int)col << " ) H: "<< horizontal;
		}

		return m_invalidBoard;
	}

	BoardPtr Board::c_advance( uint8_t player )
	{
		BoardPtr result(new Board(*this));

		if ( m_gameOver )
		{
			return result;
		}

		for(uint8_t i=0;i<Player_last;i++)
		{
			result->m_shortest_path[i] = m_shortest_path[i];
		}

		uint8_t otherPlayer = player == Player_1 ? Player_2 : Player_1;

		Coord c;
		do
		{
			if ( result->m_shortest_path[player].size() > 1 )
			{
				result->m_shortest_path[player].pop_back();
				c = result->m_shortest_path[player].back();
			}
			else
			{
				LOG_WARN(DOM)<<"FATAL ERROR: shortest path is empty";
				//result->m_isValid = false;
				//return result;

				// Finish point is occupied by oponent..
				if (getCell(c.first, c.second)->left())
				{
					--c.second;
				}
				else if (getCell(c.first, c.second)->right())
				{
					++c.second;
				}

			}
		}
		while (result->m_player_cell[otherPlayer] == c );

		result->m_player_cell[player] = c;

		result->m_lastMove.set_move_type(ADVANCE, c);
		//result->m_last_move.set_move_type(ADVANCE, result->m_player_cell[player]);

		if ( c.first == 1 or c.first == BOARD_SIZE)//result->is_player_finish_pt(c ,player))
		{
			LOG_DEBUG(DOM)<<"Found finishing move: FATALITY";

			result->m_lastMove.score = player == Player_1 ? -MAX_SCORE : MAX_SCORE;

			result->m_gameOver = true; //untested ??
		}
		else
		{
			result->m_lastMove.score = (result->m_shortest_path[Player_1].size() - result->m_shortest_path[Player_2].size());
		}

		LOG_DEBUG(DOM)<<"c_advance for player " <<(int)player << " score: " << (int)result->m_lastMove.score << "To: " << (int)c.first << ":"<< (int)c.second;

		return result;
	}

	std::vector< std::pair<Coord, CellSides> > Board::get_diff_path_cells(uint8_t player)
	{
		std::vector< std::pair<Coord, CellSides> > outVec;
		CellSides side;
		uint8_t otherPlayer = player == Player_1 ? Player_2 : Player_1;
		bool prevCellInSet = false;

		auto it = m_shortest_path[player].begin();

		while ( it != m_shortest_path[player].end() )
		{
			if (targetOnlyPathDiffs and std::find( m_shortest_path[otherPlayer].begin(), m_shortest_path[otherPlayer].end(), (*it) ) != m_shortest_path[otherPlayer].end() )
			{
				LOG_DEBUG(DOM) <<"Found common cell in path " << (int)it->first << ":" <<(int)it->second;
				prevCellInSet = false;
			}
			else
			{
				if (( not prevCellInSet ) and  (it !=  m_shortest_path[player].begin())  /*and (std::next(it,1) == m_shortest_path[player].end())*/)
				{
					it = std::prev(it, 1);
				}

				auto nxt = *(std::next(it,1));

				if ( (*it).first == nxt.first and (*it).second == (nxt.second  + 1))//( (*it)->left() ==  *(std::next(it,1)))
					side = LEFT;
				else if  ( (*it).first == nxt.first and (*it).second == (nxt.second  - 1))
					side = RIGHT;
				else if ( ((*it).first == (nxt.first+1)) and (*it).second == (nxt.second))
					side = UP;
				else if ( ((*it).first == (nxt.first-1)) and (*it).second == (nxt.second))
					side = DOWN;
				else
				{
					// Last cell after a previous revert to prev cell.
					break;
				}

				LOG_DEBUG(DOM)<<"result true on: " <<(int)(*it).first << ":" << (int)(*it).second << " - side: "<<side;

				outVec.push_back( {(*it), side} );
				prevCellInSet = true;
			}
			++it;
		}

		return outVec;
	}


	bool Board::get_diff_path_cell( uint8_t player, Coord &frontCell, CellSides &side, uint8_t index )
	{
		LOG_DEBUG(DOM)<<"start get_diff_path_cell player: "<< (int)player ;

		if (m_shortest_path[player].size() < 2)
		{
			return false;
		}

		//precached vector
		if ( m_diffVector[player].size() == 0 )
		{
			m_diffVector[player] = get_diff_path_cells(player);
		}


		if (index < m_diffVector[player].size())
		{
			frontCell = m_diffVector[player][index].first;
			side = m_diffVector[player][index].second;
			return true;
		}

		LOG_DEBUG(DOM)<<"get_diff_path_cell - not found";

		return false;
	}

#define is_parsed( c ) visited[(size_t)(c - m_board)]
#define set_parsed( c ) visited[(c - m_board)] = true


	std::list<Coord> Board::shortest_path( uint8_t player )
	{
		std::valarray<bool> visited(false, BOARD_SIZE*BOARD_SIZE + 1);

		// Parse vector holding elements of <cell, parent index position back> - used to calculate shortest path
		std::vector<std::pair<Cell*, uint8_t>> parseVec;
		parseVec.reserve(BOARD_SIZE*BOARD_SIZE + 1);

		parseVec.push_back(make_pair(getCell(m_player_cell[player].first, m_player_cell[player].second), 0));

		auto it = parseVec.begin();

		while (it != parseVec.end())
		{
			// Caution: not ok for already in endPoint ( won't be the case)

			if (it->first->left() and (not is_parsed(it->first->left())) )
			{
				set_parsed(it->first->left());

				parseVec.push_back(make_pair(it->first->left(), std::distance(it, parseVec.end()) ) );

				if ( is_player_finish_pt(it->first->left(),player))
				{
					it = std::prev(parseVec.end(), 1); // the parent of this element is the last elem in list
					break;
				}
			}
			if (it->first->up() and (not is_parsed(it->first->up())) )
			{
				set_parsed(it->first->up());

				parseVec.push_back(make_pair(it->first->up(), std::distance(it, parseVec.end()) ) );

				if ( is_player_finish_pt(it->first->up(),player))
				{
					it = std::prev(parseVec.end(), 1);
					break;
				}
			}
			if (it->first->right() and (not is_parsed(it->first->right())) )
			{
				set_parsed(it->first->right());

				parseVec.push_back(make_pair(it->first->right(), std::distance(it, parseVec.end()) ) );

				if ( is_player_finish_pt(it->first->right(),player)) // todo add define and skip check for two player game
				{
					it = std::prev(parseVec.end(), 1);
					break;
				}
			}
			if (it->first->down() and  (not is_parsed(it->first->down())) )
			{
				set_parsed(it->first->down());

				parseVec.push_back(make_pair(it->first->down(), std::distance(it, parseVec.end()) ) );
				if ( is_player_finish_pt(it->first->down(),player))
				{
					it = std::prev(parseVec.end(), 1);
					break;
				}
			}
			++it;
		}

		// Compute the shortest path back from the last element, through parent indexes (e.g.: the parent of it=<cell*, 5> is 5 elements behind it
		std::list<Coord> shortestPath;

		if (it != parseVec.end())
		{
			while (it->second != 0)
			{
				//shortestPath.push_back(it->first);

				shortestPath.push_back( {((it->first - m_board ) / BOARD_SIZE + 1), ((it->first - m_board ) % BOARD_SIZE + 1)});

				it = std::prev(it, it->second);
			}

			shortestPath.push_back(m_player_cell[player]); // Add player cell ?
		}
		else
		{
			LOG_DEBUG(DOM)<<"unable to find a path";
		}

		return shortestPath;
	}

	void Board::printCost()
	{
		LOG_DEBUG(DOM)<<"start print";
		LOG_DEBUG(DOM) << "\nplayer position: P1: " << (int)m_player_cell[Player_1].first << ":"<< (int)m_player_cell[Player_1].second << "\n  P2: "
					<< (int)m_player_cell[Player_2].first<< ":"<< (int)m_player_cell[Player_2].second ;
		stringstream ss;
		for(int i=0;i<Player_last;i++)
		{
			ss<<"Shortest path for player "<< (i+1) << " is: ";
			for (auto it : m_shortest_path[i])
			{
				ss << (int)it.first<<":"<< (int)it.second <<" - ";
			}
			ss <<"\n";
		}
		ss <<"Score: " << (int)m_lastMove.score << "\n";
		for (int i=0;i<BOARD_SIZE;i++)
		{
			for (int j=0;j<BOARD_SIZE;j++)
			{
				Cell *cpt = getCell(i+1,j+1);
				ss << "[" << (i+1) << ":" << (j+1)<< "]";
				if ( cpt->right())
					ss<< std::setw(3) <<" ";
				else
					ss <<  std::setw(3) <<"|";
			}
			ss<<"\n";
			for (int j=0;j<BOARD_SIZE;j++)
			{
				Cell *cpt = getCell(i+1,j+1);
				if ( not cpt->down())
				{
					if (not cpt->right())
					{
						ss<< std::setw(8) << "_______|";
					}
					else
						ss<< std::setw(8) << "________";
				}
				else if ( not cpt->right())
					ss<< std::setw(8) << "|";
				else
					ss<< std::setw(8) <<" ";
			}
			ss<<"\n";
		}
		LOG_DEBUG(DOM) << ss.str();
	}
}
