#include <chrono>
#include <fstream> // temp
#include "MoveInserter.h"
#include "ABotV2Analyser.h"


namespace qplugin
{


	uint16_t GlobalData::roundNumber = 0;

    ABotV2Analyser::ABotV2Analyser(qcore::PlayerId id) : m_myPlayerId(id)
    {
        LOG_INFO(DOM) << "A-Bot V2 Initialized with playerId: " << (int)m_myPlayerId;
        GlobalData::roundNumber = 0;

        qcore::PlayerState state = {qcore::Direction::Up, {8,4}, 10};
        m_initialState.setPlayerInfo(state, true);
        state = {qcore::Direction::Down, {0,4}, 10};
        m_initialState.setPlayerInfo(state, false);
    }
    
#ifdef FULL_STATE_REFRESH
    void ABotV2Analyser::setInputMap(const std::list<qcore::WallState> &currentWalls, std::vector<qcore::PlayerState> playerData)
    {
        // Log only. Debugs..
        // LOG_INFO(DOM) << "Current walls on board :";
        // for (auto &wall : currentWalls)
        //     LOG_INFO(DOM) << "(" << (int)wall.position.x <<"-"<< (int)wall.position.y <<"-" << ( wall.orientation == qcore::Orientation::Horizontal ? "h" : "v");

        m_initialState.reset();
        m_initialState.convertQCoreWalls(currentWalls);
        m_initialState.setPlayerInfo(playerData[m_myPlayerId], true);
        m_initialState.setPlayerInfo(playerData[(m_myPlayerId == 0 ? 1 : 0)], false);

        m_childrenCount = 0;

        LOG_INFO(DOM) << "setInputMap exit";
    }
#endif
#if not defined(FULL_STATE_REFRESH) || defined(DUMP_MOVES_LIST)

    void ABotV2Analyser::setOponentMove(const qcore::PlayerAction& lastAction)
    {
        if (lastAction.actionType != qcore::ActionType::Invalid)
        {
            qcore::PlayerAction lastActionD = lastAction.rotate(m_myPlayerId == 0 ? 0 : 2);
            Move opMove;

            if (lastActionD.actionType == qcore::ActionType::Move)
            {
                opMove = { {{0,0}, qcore::Orientation::Horizontal}, {lastActionD.playerPosition.x, lastActionD.playerPosition.y}, false   };
#ifndef FULL_STATE_REFRESH                
                m_initialState.moveTo(opMove.pos, false);
#endif
            }
            else
            {
                if (lastActionD.wallState.orientation == qcore::Orientation::Horizontal) 
                {
                    opMove = { {{lastActionD.wallState.position.x - 1, lastActionD.wallState.position.y}, lastActionD.wallState.orientation}, INVALID_POS, true   };
                }
                else
                {
                    opMove =  { {{lastActionD.wallState.position.x, lastActionD.wallState.position.y - 1}, lastActionD.wallState.orientation}, INVALID_POS, true   };
                }
#ifndef FULL_STATE_REFRESH       
                m_initialState.addValidWall(opMove.wall, false);
#endif

            }
#ifdef DUMP_MOVES_LIST
            m_movesHistory.push_back(opMove.flip(m_myPlayerId == 1));
#endif
        }
    }
#endif

    MoveInserterSP ABotV2Analyser::detectRunStrategy(bool)
     {
        if (GlobalData::roundNumber < 2)
            return std::make_shared<SchilerInserter>();
        else
            return std::make_shared<EverythingInserter>();
        //return std::make_shared<ExtendedHeuristicInserter>();
     }


    ABBoardCaseNode* ABotV2Analyser::minimax(ABBoardCaseNode *node, int depth, bool isMaximizingPlayer, int alpha, int beta)
	{

		if (depth == 4 || m_abortComputation || node->hasWon(!isMaximizingPlayer))
			return node;

		MoveInserter* inserter;

		if (depth <= 1)
			inserter = &m_allMovesGenerator;
		else if (depth == 2)
		 	inserter = &m_goodMovesGenerator;
		else
			inserter = &m_shortestPathMovesGenerator;

		if (isMaximizingPlayer)
		{
			ABBoardCaseNode* bestValNode = nullptr;

			for (auto &it : inserter->getMoves(node, isMaximizingPlayer))
			{
				if (node->addNewChild(it, isMaximizingPlayer))
				{
					m_childrenCount++;

					auto crtVal = minimax(node->getLastChild(),depth + 1, false, alpha, beta);

					if (bestValNode == nullptr || bestValNode->getScore() < crtVal->getScore())
					{
						bestValNode = crtVal;
					}

					alpha = std::max(alpha, (int)bestValNode->getScore());
					if (beta <= alpha)
					{
						//LOG_INFO(DOM) << "Tree cut +";
						break;
					}
				}
			}
			
#ifdef LOG_MOVES
			LOG_INFO(DOM) <<  "Best node for : [" << node->m_name << "] Maxi: " << isMaximizingPlayer << " depth: " << depth << " Score: " << (int)bestValNode->getScore() << " Name: " << bestValNode->m_name;
#endif
			
			return bestValNode;
		}
		else
		{
			ABBoardCaseNode* bestValNode = nullptr; // +INF
			for (auto &it : inserter->getMoves(node, isMaximizingPlayer))
			{
				if (node->addNewChild(it, isMaximizingPlayer))
				{
					m_childrenCount++;

					auto crtVal = minimax(node->getLastChild(),depth + 1, true, alpha, beta);

					if (bestValNode == nullptr || bestValNode->getScore() > crtVal->getScore())
					{
						bestValNode = crtVal;
					}
					beta = std::min( beta, (int)bestValNode->getScore());
					if (beta <= alpha)
					{
						//LOG_INFO(DOM) <<  "Tree cut -";
						break;
					}
				}
			}

#ifdef LOG_MOVES
			LOG_INFO(DOM) <<  "Best node for : [" << node->m_name << "] Maxi: " << isMaximizingPlayer << " depth: " << depth << " Score: " << (int)bestValNode->getScore() << " Name: " << bestValNode->m_name;
#endif

			return bestValNode;
		}
	}

    void ABotV2Analyser::trackMoveHistory(const Move& move)
    {
        #ifdef DUMP_MOVES_LIST
        m_movesHistory.push_back(move.flip(m_myPlayerId == 1));

        // bool myMove = m_myPlayerId == 0 ? true : false;
        //  std::stringstream ss;

        //     ss << "Moves history: \n";

        //     for(auto &mv : m_movesHistory)
        //     {
        //         ss << (myMove ? ">" : "<") << mv.toString() + "\n";
        //         myMove =!myMove;
        //     }

        //     LOG_INFO(DOM) << ss.str();

        #endif


        // if (m_prevPosition.size() >= 3)
		// {
		// 	m_prevPosition.pop_front();
		// }

        // if (move.isWallMove == false)
        // {
        //     m_prevPosition.push_back(move.pos);
        // }
        // else
        // {
        //     m_prevPosition.push_back(INVALID_POS);
        // }
    }

    // bool ABotV2Analyser::detectCycle(const Move& move)
    // {
    //     bool rc = false;

    //     if (move.isWallMove == false && m_prevPosition.size() == 3 )
    //     {
    //         if (m_cycleState )
    //         {
    //             LOG_ERROR(DOM) << "Still in cycle state";
    //         }

    //         rc = (m_prevPosition[0] == m_prevPosition[2]) && !(m_prevPosition[0] == INVALID_POS); //todo : optimize

    //         if (rc || m_cycleState)
    //         {
    //             LOG_ERROR(DOM) << "Posible movement cycle";

    //             rc = (m_prevPosition[1] == move.pos) && !(m_prevPosition[1] == INVALID_POS);

    //             if (rc)
    //             {
    //                 LOG_ERROR(DOM) << "Found movement cycle-loop.";
    //             }
    //         }
    //     }
    
    //     m_cycleState = rc; // if it enters a cycle state, make sure we naturaly leave it - otherwise we will just get 3+ cycle sizes instead of 2 :|

    //     return rc;
    // }

    void ABotV2Analyser::startCountdownTimer(int seconds)
    {
		stopCountdownTimer();
		m_abortComputation = false;
		m_cdThreadActive = true;

		m_countdownTimerThread =std::thread([&, seconds]()
			{
                constexpr const auto STEPS_PER_SECOND = 10;
                
				int steps = seconds * STEPS_PER_SECOND - 2;
				//LOG_INFO(DOM) << "TH: Starting new wait timer > Steps: " << steps;
				while( steps > 0 && m_cdThreadActive)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(1000 / STEPS_PER_SECOND));
					steps--;
					//LOG_INFO(DOM) << "TH: Step loop. StepVal: " << steps << " cdTHreadActive: " << m_cdThreadActive;
				}

				m_abortComputation = true;
				if (steps==0)
				{
					LOG_ERROR(DOM) << "TH: Time expired !!! ABorting";
				}
				else if (m_cdThreadActive == false)
				{
					//LOG_ERROR(DOM) << "TH: Early termination"; //finished in due time
				}
			});
    }

    void ABotV2Analyser::stopCountdownTimer()
    {
        m_cdThreadActive = false;

        if (m_countdownTimerThread.joinable())
        {
            //LOG_ERROR(DOM) << "TH: Joining";
            m_countdownTimerThread.join();
            //LOG_INFO(DOM) << "TH: Stoped timer";
        }
    }

    // Experimental. Please redo.
    Move ABotV2Analyser::computeNextMove()
    {
        GlobalData::roundNumber++;
        startCountdownTimer(SECONDS_PER_TURN);

#ifndef FULL_STATE_REFRESH
        m_initialState.clearChildren();
#endif


        // if (!m_inserter)
        // {
        //     m_inserter = detectRunStrategy(true);
        //     m_crtStrategyRemainingMoves = m_inserter->minRoundCount();
        // }

        // if (GlobalData::roundNumber < 4)
        // {
        //     LOG_INFO(DOM) << "Shiller special";
        //     if (!m_inserter)
        //         m_inserter= std::make_shared<SchilerInserter>();

        //     for (auto& mvOption : m_inserter->getMoves(&m_initialState, true))
        //     {
        //         m_initialState.addNewChild(mvOption, true);
        //     }

        //     return m_initialState.getBestChild(true)->getCurrentMove();
        // }

        auto resultNode = minimax(&m_initialState, 0, true, INT16_MIN, INT16_MAX);
        auto move = resultNode->getNextMoveFromRoot();

#if defined(FAST_TEST_MODE) || defined(DUMP_MOVES_LIST)
        static bool cycleEscapeActivated = false;// test statistics
#endif

        if (resultNode->detectCycleInTrace())
        {
            m_cycleState=2;
        }


       if (m_cycleState > 0 )
        {
            m_cycleState--;

            if (move.isWallMove == false)
            {
                LOG_INFO(DOM) <<"IN cycle !!! o_O";

                auto shortestPath = m_initialState.getShortestPath(true);

                if (shortestPath.size() > 0)
                {
                    move = {{INVALID_POS, qcore::Orientation::Horizontal}, *std::prev(shortestPath.end()), false};
#if defined(FAST_TEST_MODE) || defined(DUMP_MOVES_LIST)
                    cycleEscapeActivated = true;
#endif
                }
                else
                {
                    LOG_ERROR(DOM) <<"Exceptional error ! :O. Life=tough"; // too much !
                }
            }
            
        }


#ifndef FULL_STATE_REFRESH
        // maintain state
        if (move.isWallMove)
        {
            m_initialState.addValidWall(move.wall, true);
        }
        else
        {
            m_initialState.moveTo(move.pos, true);
        }
#endif

        // Keep move history
        trackMoveHistory(move);
        LOG_INFO(DOM) << "Children count : "<< m_childrenCount;





#if defined(FAST_TEST_MODE) || defined(DUMP_MOVES_LIST)
        bool gameOverWinning = false;
        bool gameOverLoosing = false;

        if (!m_initialState.hasWallsLeft(false) && !m_initialState.hasWallsLeft(true))
        {
            if ((m_initialState.getScore() + 100) > 0)
            {
                gameOverWinning = true;
            }
            else
            {
                gameOverLoosing = true;
            }
        }
        if (resultNode->hasWon(true) )
        {
            gameOverWinning = true;
        }
            
        if (resultNode->hasWon(false))
        {
            gameOverLoosing = true;
        }

#ifdef DUMP_MOVES_LIST
        if (gameOverLoosing)
        {
             //m_movesList
            bool myMove = m_myPlayerId == 0 ? true : false;

            std::stringstream ss;

            ss << "Moves history: \n";

            for(auto &mv : m_movesHistory)
            {
                ss << (myMove ? ">" : "<") << mv.toString() + "\n";
                myMove =!myMove;
            }

            LOG_INFO(DOM) << ss.str();

            if (cycleEscapeActivated)
            {
                srand(time(NULL));
                std::ofstream dumFile ("Cycle_LossMovesList_" + std::to_string(rand()));
                dumFile << ss.str();

                dumFile.close();
            }
            
        }
#endif   //DUMP_MOVES_LIST

#ifdef FAST_TEST_MODE
        if (gameOverWinning)
        {

            std::ofstream logF("wins", std::ios::app);
            logF << ( m_myPlayerId == 0 ? ">":"<") <<  m_initialState.getScore() << (cycleEscapeActivated ? " wCE" : " ko") <<"\n";

            logF.close();

            exit(0);
        }
        else if (gameOverLoosing)
        {
            std::ofstream logF("losses", std::ios::app);
            logF <<( m_myPlayerId == 0 ? ">":"<") <<  m_initialState.getScore() << (cycleEscapeActivated ? " wCE" : " ko")  << "\n";

            logF.close();

            exit(0);
        }
#endif //FAST_TEST_MODE
#endif    // defined(FAST_TEST_MODE) || defined(DUMP_MOVES_LIST)


        return move;
    }
         
}