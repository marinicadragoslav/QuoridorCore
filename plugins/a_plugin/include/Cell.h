/*
 * Cell.h
 *
 *  Created on: Feb 18, 2020
 *      Author: andrei
 */

#ifndef PLUGINS_A_PLUGIN_INCLUDE_CELL_H_
#define PLUGINS_A_PLUGIN_INCLUDE_CELL_H_

#include <string>

#include "Defines.h"


namespace TermAi
{

	class Cell
	{
		public:
		uint8_t neighbours;

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
		inline void down(bool enable)
		{
			neighbours ^= (-enable ^ neighbours) & ( DOWN);
		}

		inline Cell* left()
		{
			return (neighbours & LEFT ? this - 1 : nullptr);
		}
		inline Cell *up()
		{
			return (neighbours & UP ? this - BOARD_SIZE : nullptr);
		}
		Cell *right()
		{
			return (neighbours & RIGHT ? this + 1 : nullptr);
		}
		inline Cell *down()
		{
			return (neighbours & DOWN ? this + BOARD_SIZE : nullptr);
		}
	};
}



#endif /* PLUGINS_A_PLUGIN_INCLUDE_CELL_H_ */
