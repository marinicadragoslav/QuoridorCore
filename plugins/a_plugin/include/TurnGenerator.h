/*
 * TurnGenerator.h
 *
 *  Created on: May 3, 2020
 *      Author: andrei
 */

#ifndef PLUGINS_A_PLUGIN_INCLUDE_TURNGENERATOR_H_
#define PLUGINS_A_PLUGIN_INCLUDE_TURNGENERATOR_H_

#include <vector>
#include <set>

#include "Board.h"


namespace TermAi
{

class TurnGenerator;
typedef std::shared_ptr<TurnGenerator> TurnGeneratorPtr;

/*
 * TurnGenerator class represents a node from the solution tree.
 *   This is a n-tree with each level representing options a player (e.g.: lvl 1: self, lvl 2 oponent, lvl 3 self etc).
 *   Each node represents posible move options for a given board situation. From each option several other posible moves are generated for the oponent and so on.
 *   	Optimizations:
 *   			* only moves generating the max score are kept (always looking for immediate advantage - as keeping all generates too many nodes)
 *   			* on "Easy" levels: algorithm tries to only target cells that affect only the oponent's shortest path but not his own. This sometimes skips too many solutions
 *
 * */
class TurnGenerator
	{
	public:
		// Ctor from current board state
		TurnGenerator(BoardPtr b, uint8_t player, uint8_t wallsLeftP1, uint8_t wallsLeftP2);

		~TurnGenerator();

		void compute();

		// Get the result and the expected future moves
		std::list<Move> get_moves();

	private:
		// Private ctor for generating childen nodes
		TurnGenerator(TurnGenerator* parent, BoardPtr b, uint8_t childIdx);

		// Add move to posible solutions
		bool add_move( BoardPtr generatedMove );

		// Report solution to parent
		void report_finished( int16_t childIndex );

		// Start process node (todo: create thread pool for parallel processing since tree branches are not dependent + sync parent reporting)
		void add_pending_processing( TurnGeneratorPtr move );

		// Start to process nodes on @recLvl level
		void process_depth_level(uint8_t recLvl, std::list<TurnGeneratorPtr> moveList);

		// Re-arange child indexes
		void restore_child_indexes();

		// Parse solution tree and pick the best cantidate
		//   note: self will maximize and oponent will minimize score) [Function might have some bugs :) ]
		TurnGenerator* get_best_gen();

		// Add a move to the solution vector that will prevent the move of the child @childIndex
		void handle_prevent_move( Move &bestMove);

		// Debug funtion showing the solution tree
		void print_gen_tree(int recLvl, TurnGenerator* obj);


		// Members:

		// Inherited board; present situation
		BoardPtr m_initialBoard;

		uint8_t m_player;

		// List of generated solutions
		std::list<TurnGeneratorPtr> m_moveList;


		std::vector<bool> m_childComputed;

		// Computing on behalf of self or the oponent
		bool m_self;

		// Own identifier inside the parent node
		int16_t m_indexInParent;

		// Parent in generation tress
		TurnGenerator* m_parent;

		// Walls left for self
		uint8_t m_ownWallsLeft;

		// Walls left for oponent
		uint8_t m_opWallsLeft;

		static int m_currentDepth; // level of processing

		// "Hash" of the node - to avoid duplicates
		std::set<int> m_createdNodes;

		//helpers
		std::string m_name; //dbug only

	};

}




#endif /* PLUGINS_A_PLUGIN_INCLUDE_TURNGENERATOR_H_ */
