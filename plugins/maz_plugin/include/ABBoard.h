#ifndef Header_qcore_BOARD_H
#define Header_qcore_BOARD_H

#include "BaseData.h"

namespace qplugin
{
    enum CellSides
	{
		LEFT = 1,
		UP = 2,
		RIGHT = 4,
		DOWN = 8,

        WALL_START = 16 // identifier for the first part of the wall (as vertical walls can be placed BETWEEN horizontal walls and vice-versa)
	};


    // Class entity optimized for static linear array to be viewed as unweighted graph
    class Cell
	{
		public:
		uint8_t neighbours;

		Cell(uint8_t fromval) : neighbours(fromval){}

		operator int() { return neighbours;}

		Cell():neighbours(0) {}

		inline void left(bool enable)
		{
			neighbours ^= (-enable ^ neighbours) & ( LEFT);
		}
		inline void up(bool enable)
		{
			neighbours ^= (-enable ^ neighbours) & ( UP);
		}
		inline void right(bool enable)
		{
			neighbours ^= (-enable ^ neighbours) & (RIGHT);
		}
		inline void rightSt(bool enable)
		{
			//neighbours ^= (-enable ^ neighbours) & (RIGHT);
			neighbours = enable ? (neighbours | RIGHT) : (neighbours & (~RIGHT ));
			neighbours = !enable ? (neighbours | WALL_START) : (neighbours & (~WALL_START ));
		}
		inline void down(bool enable)
		{
			neighbours ^= (-enable ^ neighbours) & ( DOWN);
		}
        inline void downSt(bool enable)
		{
			neighbours = enable ? (neighbours | DOWN) : (neighbours & (~DOWN ));
			neighbours = !enable ? (neighbours | WALL_START) : (neighbours & (~WALL_START ));
		}

		inline Cell* left()
		{
			return (neighbours & LEFT ? this - 1 : nullptr);
		}
		inline Cell *up()
		{
			return (neighbours & UP ? this - qcore::BOARD_SIZE : nullptr);
		}
		Cell *right()
		{
			return (neighbours & RIGHT ? this + 1 : nullptr);
		}
		inline Cell *down()
		{
			return (neighbours & DOWN ? this + qcore::BOARD_SIZE : nullptr);
		}

		inline bool isWallStart() {  return (neighbours & WALL_START);}
	};
    
    
    class ABBoard//inner class - not secure
    {
        public:
            ABBoard();
			~ABBoard() = default;

			ABBoard(const ABBoard& src) { memcpy(m_board, src.m_board, sizeof(m_board));}

			void operator=(const ABBoard& src)  { memcpy(m_board, src.m_board, sizeof(m_board));}
            
            //debug only.
            void printState();

			void placeValidWall(qcore::Position pos, bool horizontal);

			// Slightly faster as is only calculates the len
			int shortestPathLen(qcore::Position pos, qcore::Direction endPoint);

			// Gets the shortest path coords
			std::vector<qcore::Position> shortestPath(qcore::Position pos, qcore::Direction endPoint);

			bool wallAllowed(qcore::Position pos, bool horizontal);

			 std::vector<qcore::WallState> getWalls();

			inline bool moveAllowed(qcore::Position pos,  qcore::Direction direction) // must be inside the board !
				{  
					return 
						// (direction == qcore::Direction::Up && (Pos2CellId(pos.x,pos.y)  >= qcore::BOARD_SIZE) && getCell(pos.x, pos.y)->up()) ||
						// (direction == qcore::Direction::Right && ( Pos2CellId(pos.x, pos.y) % qcore::BOARD_SIZE ) != (qcore::BOARD_SIZE-1)  && getCell(pos.x, pos.y)->right()) ||
						// (direction == qcore::Direction::Down && Pos2CellId(pos.x, pos.y) < (qcore::BOARD_SIZE * (qcore::BOARD_SIZE-1)) && getCell(pos.x, pos.y)->down() ) ||
						// (direction == qcore::Direction::Left && (Pos2CellId(pos.x, pos.y) % qcore::BOARD_SIZE) != 0 && getCell(pos.x, pos.y)->left());
						(direction == qcore::Direction::Up && getCell(pos.x, pos.y)->up()) ||
						 (direction == qcore::Direction::Right && getCell(pos.x, pos.y)->right()) ||
						 (direction == qcore::Direction::Down && getCell(pos.x, pos.y)->down() ) ||
						 (direction == qcore::Direction::Left && getCell(pos.x, pos.y)->left());
				  }
			inline bool moveAllowed(Position oldPos, Position newPos)
			{
				return 
					oldPos.x > newPos.x ? moveAllowed(oldPos,  qcore::Direction::Up) :
						(oldPos.x<newPos.x ?  moveAllowed(oldPos,  qcore::Direction::Down) :
							(oldPos.y > newPos.y ?  moveAllowed(oldPos,  qcore::Direction::Left) : moveAllowed(oldPos,  qcore::Direction::Right)));
			}
        private:
        
        // Actual board
		Cell m_board[qcore::BOARD_SIZE * qcore::BOARD_SIZE] = 
            { 
                12, 13, 13, 13, 13, 13, 13, 13,  9, 
                14, 15, 15, 15, 15, 15, 15, 15, 11,
                14, 15, 15, 15, 15, 15, 15, 15, 11,
                14, 15, 15, 15, 15, 15, 15, 15, 11,
                14, 15, 15, 15, 15, 15, 15, 15, 11,
                14, 15, 15, 15, 15, 15, 15, 15, 11,
                14, 15, 15, 15, 15, 15, 15, 15, 11,
				14, 15, 15, 15, 15, 15, 15, 15, 11,
                 6,  7,  7,  7,  7,  7,  7,  7,  3
                
            };
        
        private: // functions
        
        // Warning ! no overflow check.
        inline constexpr Cell* getCell(uint8_t row, uint8_t col) { return &m_board[row * qcore::BOARD_SIZE + col]; }

		inline constexpr bool cellIsLastUpperRow(Cell* cell) { return ((cell-m_board) < qcore::BOARD_SIZE);}

		inline constexpr bool cellIsLastDownerRow(Cell* cell) { return ((cell-m_board) >= (qcore::BOARD_SIZE * (qcore::BOARD_SIZE - 1))); }

		inline constexpr int getCellLinearIndex(Cell* cell) { return (cell - m_board); } // first cell = 0, 7th=7, [1][0] = 9 etc
    };
    

	// Board implementation, calculating data on the fly without any graph or matrix. Each cell has it's own unque idetifier.

	#define Pos2CellId(x,y) (y + x *qcore::BOARD_SIZE)

	#define downV2(x) (x + qcore::BOARD_SIZE)
	#define upV2(x) (x - qcore::BOARD_SIZE)
	#define rightV2(x) (x + 1)
	#define leftV2(x) (x - 1)

    class ABBoardv2//inner class - not secure
    {
        public:
            ABBoardv2();
            
            //debug only.
            void printState();

			void placeValidWall(qcore::Position pos, bool horizontal);

			// Slightly faster as is only calculates the len
			int shortestPathLen(qcore::Position pos, qcore::Direction endPoint);

			// Gets the shortest path coords
			std::vector<qcore::Position> shortestPath(qcore::Position pos, qcore::Direction endPoint);

			bool wallAllowed(qcore::Position pos, bool horizontal);

			bool moveAllowed(qcore::Position pos,  qcore::Direction direction);

			inline bool moveAllowed(Position oldPos, Position newPos)
			{
				return 
					oldPos.x > newPos.x ? moveAllowed(oldPos,  qcore::Direction::Up) :
						(oldPos.x<newPos.x ?  moveAllowed(oldPos,  qcore::Direction::Down) :
							(oldPos.y > newPos.y ?  moveAllowed(oldPos,  qcore::Direction::Left) : moveAllowed(oldPos,  qcore::Direction::Right)));
			}

			private:
			std::vector<uint8_t> m_horizontalWalls;
			std::vector<uint8_t> m_VerticalWalls;
	};

}

#endif // Header_qcore_BOARD_H
