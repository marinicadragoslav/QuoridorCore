/*
 * Board.h
 *
 *  Created on: Feb 18, 2020
 *      Author: andrei
 */

#ifndef PLUGINS_A_PLUGIN_INCLUDE_BOARD_H_
#define PLUGINS_A_PLUGIN_INCLUDE_BOARD_H_

#include <memory>

#include "Cell.h"
#include "BoardState.h"
#include "Player.h"

namespace TermAi
{

enum MoveType
{
	INVALID,
	ADVANCE,
	PLACE_WALL_H,
	PLACE_WALL_V
};

typedef std::pair<uint8_t, uint8_t> Coord;

class Move
{
public:
	Move();

	void set_move_type(MoveType type, Coord pos);

	Coord m_location;
	MoveType moveType;
	int8_t score;
};


	class Board;

	typedef std::shared_ptr<Board> BoardPtr;



	class Board
	{
	public:
		// Construct empty board
		Board();

		// Copy ctor - for speed-up - since not all data needs to be copied
		Board(const Board &b2);

		// Create board from qcore board
		Board(std::vector<qcore::PlayerState> players,
				std::list<qcore::WallState> walls);

		// Get cells to process.
		//   for targetOnlyPathDiffs=true - will return only cells that belong to the oponent's shortest path (for speed)
		//   for targetOnlyPathDiffs= false - will return each cell of the oponent's shortest path
		//         (targetOnlyPathDiffs == true) - is fast but has poor results. Idea: dynamic switching on deeper levels
		bool get_diff_path_cell( uint8_t player, Coord &frontCell, CellSides &side, uint8_t index);

		bool m_isValid; // create inline func.

		// And only
		Move m_lastMove;

		static bool targetOnlyPathDiffs; // todo: create sync. config setter

		// Create another board from the current board by placing a wall
		BoardPtr c_place_wall(uint8_t row, uint8_t col, bool horizontal);

		// Create new board from current board + advance
		BoardPtr c_advance( uint8_t player );

		// Debug function.
		void printCost();


	private:

		// Set board boundaries
		void update_links();

		inline Cell* getCell(uint8_t row, uint8_t col);

		// Get cells to process (get result vector - for caching)
		std::vector< std::pair<Coord, CellSides> > get_diff_path_cells(uint8_t player);

		// Place a wall on the existing object
		void place_wall(uint8_t row, uint8_t col, bool horizontal);

		// Can place wall (has flaw, but will leave as it is for now..)
		bool can_place_wall(uint8_t row, uint8_t col, bool horizontal);

		// Place a wall on the existing object
		void place_wall(Cell *corner, bool horizontal);

		// Compute the shortest path on the current object for the given player
		std::list<Coord> shortest_path(uint8_t player);

		// Cell is finish point
		inline bool is_player_finish_pt(Cell *c, uint8_t player);

		// Refresh path info after placing a wall
		void refresh_info();

		//--------------
		//members
		//--------------

		// Actual board
		Cell m_board[BOARD_SIZE * BOARD_SIZE];

		// Player position on board
		Coord m_player_cell[Player_last];

		// Player's shortest path
		std::list<Coord> m_shortest_path[Player_last];

		// *_*
		bool m_gameOver;

		// cells in Player<idx>'s path that do not bellong to the other player's path (or not :) - deppending on targetOnlyPathDiffs var)
		std::vector< std::pair<Coord, CellSides> > m_diffVector[Player_last];

		// cached constant
		static BoardPtr m_invalidBoard;
	};
}



#endif /* PLUGINS_A_PLUGIN_INCLUDE_BOARD_H_ */
