#include <thread>
#include <cassert>

#include "PluginMain.h"
#include "QcoreUtil.h"



using namespace qcore::literals;
using namespace std::chrono_literals;

// #define MAXITER 20000
// #define MAXSECONDS 5

namespace qplugin
{
   /** Log domain */
   const char * const DOM = "qplugin::AB2";

   ABotV2::ABotV2(qcore::PlayerId id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game),
      m_analyser(id)
   {
      
   }

   void ABotV2::doNextMove()
   {
      bool result = false;
      LOG_INFO(DOM) << "Player " << (int)getId() << " is meditating.. >>>  ";


      // NOTE !!! Own start position is always the same (8,4) - regardless of playerId. 
   
#ifdef FULL_STATE_REFRESH
      m_analyser.setInputMap(getBoardState()->getWalls(getId()),getBoardState()->getPlayers(getId()));
#endif
# if not defined(FULL_STATE_REFRESH) || defined(DUMP_MOVES_LIST)
      m_analyser.setOponentMove(getBoardState()->getLastAction());
#endif
      
      Move nextMove = m_analyser.computeNextMove();

      // Transform internal move to a qcore move
      if (nextMove.isWallMove)
      {
         qcore::WallState apiWall = {nextMove.wall.position, nextMove.wall.orientation};

         if (apiWall.orientation == qcore::Orientation::Horizontal)
			{
				apiWall.position = apiWall.position + 1_x;
			}
			else
			{
				apiWall.position = apiWall.position + 1_y;
			}

         result = placeWall(apiWall);
      }
      else
      {
         auto NewPos = nextMove.pos;
         result = move(NewPos);
      }

      if (result == false)
      {
         //assert(false); //todo remove..
         LOG_ERROR(DOM) << "ERR !! Moving up ! Or somwhere..";
         move (qcore::Direction::Up) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Down) ;
      }

      // Close waiting thread in the oponent's turn.. (does  not consume rss - only a wait timeout)
      m_dummyFuture = std::async(std::launch::async, [this]{  m_analyser.stopCountdownTimer(); });
      
      LOG_DEBUG(DOM) << "doNextMove Exit";
      //move(qcore::Direction::Down) or move(qcore::Direction::Left) or move(qcore::Direction::Right) or move(qcore::Direction::Up);
   }

} // namespace qplugin
