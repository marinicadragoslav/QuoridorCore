
#include <chrono>
#include <thread>
#include <algorithm>

#include "QcoreUtil.h"

#include "APlugin.h"
#include "Board.h"
#include "TurnGenerator.h"


#include <iostream>

using namespace qcore::literals;

namespace qplugin
{
   /** Log domain */
   const char * const DOM = "qplugin::Term";

   A_Plugin::A_Plugin(uint8_t id, const std::string& name, qcore::GamePtr game) :
      qcore::Player(id, name, game)
   {
   }

   void A_Plugin::doNextMove()
  {
    LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking ..";

    // Leave mess as it is, since it was designed to be generic but it's too complicated to untangle now: AI will always be player2 and heading down
    TermAi::TurnGeneratorPtr turn;

    if (getId() == TermAi::Player_1)
    {
 	   auto pVec = getBoardState()->getPlayers(1);
 	   std::reverse(std::begin(pVec), std::end(pVec));

 	   TermAi::BoardPtr crtBoard(new TermAi::Board(pVec, getBoardState()->getWalls(1)));

 	   turn = TermAi::TurnGeneratorPtr(new TermAi::TurnGenerator(
 				crtBoard, TermAi::Player_2, getBoardState()->getPlayers(0)[TermAi::Player_1].wallsLeft, getBoardState()->getPlayers(0)[TermAi::Player_2].wallsLeft)
 		);
    }
    else
	{
		TermAi::BoardPtr crtBoard(new TermAi::Board(getBoardState()->getPlayers(0), getBoardState()->getWalls(0)));

		turn = TermAi::TurnGeneratorPtr(new TermAi::TurnGenerator(
				crtBoard, getId(), getBoardState()->getPlayers(0)[TermAi::Player_2].wallsLeft, getBoardState()->getPlayers(0)[TermAi::Player_1].wallsLeft)
		);
	}

	turn->compute();

	std::list<TermAi::Move> decidedMoves = turn->get_moves();

	TermAi::Move nextMove = *decidedMoves.begin();

	if (nextMove.moveType == TermAi::ADVANCE)
	{
		std::cout<<"advancing.."<<std::endl;

		move(8 - (nextMove.m_location.first - 1), 8 - (nextMove.m_location.second-1));
	}
	else if (nextMove.moveType == TermAi::PLACE_WALL_H)
	{
		placeWall(8 - (nextMove.m_location.first - 1), 8 - (nextMove.m_location.second), qcore::Orientation::Horizontal);
	}
	else
	{
		placeWall(8 - nextMove.m_location.first, 8 - (nextMove.m_location.second - 1), qcore::Orientation::Vertical);
	}


	//cout << "exit get_next_mv" <<endl;
  }
} // namespace qplugin
