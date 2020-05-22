/*
 * Defines.h
 *
 *  Created on: Feb 18, 2020
 *      Author: andrei
 */

#ifndef PLUGINS_A_PLUGIN_INCLUDE_DEFINES_H_
#define PLUGINS_A_PLUGIN_INCLUDE_DEFINES_H_


namespace TermAi
{
	constexpr int BOARD_SIZE = 9;
	#define MAX_SCORE 100

	enum CellSides
	{
		LEFT = 1,
		UP = 2,
		RIGHT = 4,
		DOWN = 8
	};

	enum Players
	{
	    Player_1,
	    Player_2,
	    Player_last
	};

	/** Log domain */
	const char * const DOM = "qplugin::ATerm";

}

//#define LVL_VERY_EASY
//#define LVL_EASY
#define LVL_MEDIUM
//#define LVL_HARD





#ifdef LVL_VERY_EASY
	#define MAX_DEPTH 2
	#define ONLY_PATH_DIFFS true
#endif

#ifdef LVL_EASY
	#define MAX_DEPTH 4
	#define ONLY_PATH_DIFFS true
#endif

#ifdef LVL_MEDIUM
	#define MAX_DEPTH 4
	#define ONLY_PATH_DIFFS false
#endif

#ifdef LVL_HARD
	#define MAX_DEPTH 6
	#define ONLY_PATH_DIFFS false
#endif



#endif /* PLUGINS_A_PLUGIN_INCLUDE_DEFINES_H_ */
