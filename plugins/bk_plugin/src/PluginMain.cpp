#include <thread>
#include <cassert>

#include "PluginMain.h"
#include "QcoreUtil.h"



using namespace qcore::literals;
using namespace std::chrono_literals;

#define MAXITER 200000
#define MAXSECONDS 4 // somethimes it is not working as expected...

namespace qplugin
{
   /** Log domain */
   const char * const DOM = "qplugin::BK_Plugin";

   BK_Plugin::BK_Plugin(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {  
      if (getId() == 0)
      {
         game_tree = new MCTS_tree(new Quoridor_state(false));
         state = new Quoridor_state(false);
      }
      else
      {
         game_tree = new MCTS_tree(new Quoridor_state(true));
         state = new Quoridor_state(true);
      }
   }

   void BK_Plugin::doNextMove()
   {

      bool result = false;
      LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking.. >>>";

      qcore::PlayerAction lastMove = getBoardState()->getLastAction();

      Quoridor_move prevMove = {0,0,0,0};
      prevMove.player = 'W'; // getId() == 0 ? 'B': 'W';
      if (lastMove.actionType != qcore::ActionType::Invalid)
      {
         if (lastMove.actionType == qcore::ActionType::Wall)
         {  
            if (getId() == 0)
            {
               prevMove.x = lastMove.wallState.position.x;
               prevMove.y = lastMove.wallState.position.y;
            }
            else
            {
               prevMove.x = (int)lastMove.wallState.rotate(2).position.x;
               prevMove.y = (int)lastMove.wallState.rotate(2).position.y;
            }

            if (lastMove.wallState.orientation == qcore::Orientation::Horizontal)
            {
               prevMove.type = 'h';
               prevMove.x--;
            }  
            else
            {
               prevMove.type = 'v';
               prevMove.y--;
            }
               

            LOG_INFO(DOM)<< "Applying last move as wall on: " << prevMove.x << "-" << prevMove.y <<" O: " <<  prevMove.type;
         }
         else
         {
            if (getId() == 0)
            {
               prevMove.x = lastMove.playerPosition.x;
               prevMove.y = lastMove.playerPosition.y;
            }
            else
            {
               prevMove.x = (int)lastMove.rotate(2).playerPosition.x;
               prevMove.y = (int)lastMove.rotate(2).playerPosition.y;
            }

            LOG_INFO(DOM)<< "Applying last move as move to: " << prevMove.x << "-" << prevMove.y ;
         }

         game_tree->advance_tree(&prevMove);
         assert(state->play_move(&prevMove));
      }
      else
      {
         LOG_INFO(DOM)<< "No prev move";
      }
     
  
      LOG_ERROR(DOM)<< "Whose turn: " << state->whose_turn() << " walls avlb: " << state->remaining_walls(state->whose_turn());

      // grow tree by thinking ahead and sampling monte carlo rollouts
      auto timeAvailable = state->remaining_walls(state->whose_turn()) > 0 ? MAXSECONDS : 1;
      LOG_ERROR(DOM)<< "timeAvailable" << timeAvailable;

      game_tree->grow_tree(MAXITER, timeAvailable);
      game_tree->print_stats();   // debug

      // select best child node at root level
      MCTS_node *best_child = game_tree->select_best_child();
      LOG_ERROR(DOM)<< "____________________\n  SOLUTION Move: \n______________________";
      best_child->print_stats();
      if (best_child == NULL) 
      {
         LOG_INFO(DOM)<<  "Warning: Could not find best child. Tree has no children? Possible terminal node" ;
      }
      const Quoridor_move *best_move = (const Quoridor_move *) best_child->get_move();

      // advance the tree so the selected child node is now the root
      game_tree->advance_tree(best_move);

      // play AI move
      bool succ = state->play_move(best_move);
      if (!succ) 
      {
         LOG_INFO(DOM)<< "Warning: AI generated illegal move: ";
         best_move->sprint();
         assert(false);
      } 
      else 
      {
         // print AI's move
         cout << best_move->sprint() << endl << endl;
      }

	// else if (nextMove.moveType == TermAi::PLACE_WALL_H)
	// {
	// 	placeWall(8 - (nextMove.m_location.first - 1), 8 - (nextMove.m_location.second), qcore::Orientation::Horizontal);
	// }
	// else
	// {
	// 	placeWall(8 - nextMove.m_location.first, 8 - (nextMove.m_location.second - 1), qcore::Orientation::Vertical);
	// }



      if (best_move->type == 'h')
      {  qcore::WallState apiWall = {{(int8_t)best_move->x, (int8_t)best_move->y}, qcore::Orientation::Horizontal};
         apiWall.position = apiWall.position + 1_x;
			
         result = placeWall(apiWall);
      }
      else if (best_move->type == 'v')
      {
         qcore::WallState apiWall = {{(int8_t)best_move->x, (int8_t)best_move->y}, qcore::Orientation::Vertical};
         apiWall.position = apiWall.position + 1_y;
			
         result = placeWall(apiWall);
      }
      else
      {  
         result = move(best_move->x, best_move->y);

         if (result == false)
         {
               // LOG_INFO(DOM)<< "Warning: AI generated illegal move: ";
               LOG_ERROR(DOM) << "ERR move !! Moving up !"<<best_move->x<<best_move->y;
               move (qcore::Direction::Up);
         }
      }
      
      //LOG_WARN(DOM) << "Making a random move.";
      //move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }

} // namespace qplugin
