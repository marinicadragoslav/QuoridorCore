#include <sstream>
#include <algorithm>
#include <thread> //tmp

#include "ABBoard.h"


namespace qplugin
{
	

   ABBoard::ABBoard()
   {
       //LOG_INFO(DOM) << "ABBoard constucted.";

		/*
	    //Important !!
		LOG_INFO(DOM) << "Corners: " << (int)(*getCell(0,0)) << "-" << (int)(*getCell(0,8)) << "-" << (int)(*getCell(8,8)) << "-" << (int)(*getCell(8,0)); // 12-9-3-6

	   placeValidWall({0,0}, true);  //api coords: {1,0}
	   placeValidWall({0,7}, true);  // api coords: {1,7}
	   placeValidWall({7,0}, true);  //api coords: {8,0}
	   placeValidWall({7,7}, true);  // api coords: {8,7}

	   placeValidWall({0,0}, false); //api coords: {0,1}
	   placeValidWall({0,7}, false); //api coords: {0,8}
	   placeValidWall({7,0}, false); //api coords: {7,1}
	   placeValidWall({7,7}, false); //api coords: {7,8}
       printState();*/
   }
   
   void ABBoard::printState()
   {
       LOG_DEBUG(DOM)<<"Start print";
       std::stringstream ss;

	   ss <<"\n\n";
       for (int i=0;i<qcore::BOARD_SIZE;i++)
		{
			for (int j=0;j<qcore::BOARD_SIZE;j++)
			{
				Cell *cpt = getCell(i,j);

				ss << "[" << (i+1) << ":" << (j+1)<< "]";
				if ( cpt->right())
					ss<< std::setw(3) <<" ";
				else if (cpt->isWallStart())
					ss <<  std::setw(3) <<"I";
				else
					ss <<  std::setw(3) <<"|";
			}
			ss<<"\n";
			for (int j=0;j<qcore::BOARD_SIZE;j++)
			{
				Cell *cpt = getCell(i,j);
				if ( not cpt->down())
				{
					if (not cpt->right() && cpt->isWallStart())
					{
						ss<< std::setw(8) << "=======I";
					}
					else if (not cpt->right())
					{
						ss<< std::setw(8) << "-------|";
					}
					else if(cpt->isWallStart())
						ss<< std::setw(8) << "========";
					else
						ss<< std::setw(8) << "--------";
				}
				else if ( not cpt->right() && cpt->isWallStart())
					ss<< std::setw(8) << "I";
				else if ( not cpt->right())
					ss<< std::setw(8) << "|";
				else
					ss<< std::setw(8) <<" ";
			}
			ss<<"\n";
		}
		LOG_INFO(DOM) << ss.str();
   }
   

   void ABBoard::placeValidWall(qcore::Position pos, bool horizontal)
   {
		if (horizontal)
		{
			getCell(pos.x, pos.y)->downSt(false); // mark wall start point (since walls can be placed between other 2 walls of different orientations)
			getCell(pos.x+1, pos.y)->up(false);
			getCell(pos.x, pos.y+1)->down(false);
			getCell(pos.x+1, pos.y+1)->up(false);
		}
		else
		{
			getCell(pos.x, pos.y)->rightSt(false);
			getCell(pos.x, pos.y+1)->left(false); // leftSt ? - check on canPlaceWall
			getCell(pos.x+1, pos.y)->right(false);
			getCell(pos.x+1, pos.y+1)->left(false);
		}
   }

   int ABBoard::shortestPathLen(qcore::Position pos, qcore::Direction endPoint)
   {
		if ((endPoint == qcore::Direction::Up && cellIsLastUpperRow(getCell(pos.x, pos.y))) ||
			(endPoint == qcore::Direction::Down && cellIsLastDownerRow(getCell(pos.x, pos.y))))
		{
			return -100;
		}
			

		std::array<bool, qcore::BOARD_SIZE*qcore::BOARD_SIZE> visited = {false}; // visited hash
		//std::vector<Cell*> parseVec; //parse queue  // todo: stack allocator ? array + size idx
		//parseVec.reserve(qcore::BOARD_SIZE*qcore::BOARD_SIZE + 1);
		std::array<Cell*, qcore::BOARD_SIZE*qcore::BOARD_SIZE>  parseVec;
		int parseVecSize = 1;
		
		parseVec[0] = getCell(pos.x, pos.y);
		visited[getCellLinearIndex(parseVec.front())] = true;
		int currentDistance = 0;
		int nextDistanceLevelSwitch = 0;
		int curentIndexInParseVec = 0;

		while(curentIndexInParseVec < parseVecSize)
		{
			Cell* &crtCell = parseVec[curentIndexInParseVec];
			
			// dbg only:
			//LOG_DEBUG(DOM) << "Current cell: [" << (getCellLinearIndex(crtCell) / qcore::BOARD_SIZE + 1)  <<":" << (getCellLinearIndex(crtCell) % qcore::BOARD_SIZE + 1) <<  "]";

			if ((endPoint == qcore::Direction::Up && parseVec[curentIndexInParseVec]->up() && cellIsLastUpperRow(parseVec[curentIndexInParseVec]->up())) or // split iinto 2 functions for faster speed ??
				(endPoint == qcore::Direction::Down && parseVec[curentIndexInParseVec]->down() && cellIsLastDownerRow(parseVec[curentIndexInParseVec]->down())))
				{
					// reached end
					//LOG_DEBUG(DOM) << "reached end. distance : " << currentDistance;
					//break;
					//LOG_INFO(DOM) << "Shortest path from [" << pos.x + 1 << ":" << pos.y + 1 <<" ]: "<< currentDistance + 1;
					return currentDistance + 1;
				}
			
			if (crtCell->up() && !visited[getCellLinearIndex(crtCell->up())] ) // split optimization...
			{
				visited[getCellLinearIndex(crtCell->up())] = true;
				parseVec[parseVecSize++] = crtCell->up();
			}
				
			if (crtCell->left() && !visited[getCellLinearIndex(crtCell->left())])
			{
				visited[getCellLinearIndex(crtCell->left())] = true;
				parseVec[parseVecSize++] = crtCell->left();
			}
				
			if (crtCell->right() && !visited[getCellLinearIndex(crtCell->right())])
			{
				visited[getCellLinearIndex(crtCell->right())] = true;
				parseVec[parseVecSize++] = crtCell->right();
			}
				
			if (crtCell->down() && !visited[getCellLinearIndex(crtCell->down())])
			{
				visited[getCellLinearIndex(crtCell->down())] = true;
				parseVec[parseVecSize++] = crtCell->down();
			}

			if (curentIndexInParseVec == nextDistanceLevelSwitch)
			{
				currentDistance++;
				nextDistanceLevelSwitch = parseVecSize - 1;
				//LOG_DEBUG(DOM) << "Distance increased at: " << currentDistance << " next switch : " << nextDistanceLevelSwitch;
			}
				
			curentIndexInParseVec++;
			//LOG_DEBUG(DOM) << "moving to next idx: " << curentIndexInParseVec;
		}

		return -1;
   }

   std::vector<qcore::Position> ABBoard::shortestPath(qcore::Position pos, qcore::Direction endPoint)
   {
		//LOG_DEBUG(DOM) << "Enter shortest path" ;
		std::vector<qcore::Position> shortestPath;

		if ((endPoint == qcore::Direction::Up && cellIsLastUpperRow(getCell(pos.x, pos.y))) ||
			(endPoint == qcore::Direction::Down && cellIsLastDownerRow(getCell(pos.x, pos.y))))
		{
			return shortestPath;
		}

		std::array<bool, qcore::BOARD_SIZE*qcore::BOARD_SIZE> visited = {false}; // visited hash
		std::array<std::pair<Cell*, short>, qcore::BOARD_SIZE*qcore::BOARD_SIZE>  parseVec;
		int parseVecSize = 1;
		
		
		parseVec[0] = {getCell(pos.x, pos.y), -1};
		visited[getCellLinearIndex(parseVec.front().first)] = true;
		int curentIndexInParseVec = 0;

		while(curentIndexInParseVec < parseVecSize)
		{
			Cell* &crtCell = parseVec[curentIndexInParseVec].first;
			
			// dbg only:
			//LOG_DEBUG(DOM) << "Current cell: [" << (getCellLinearIndex(crtCell) / qcore::BOARD_SIZE + 1)  <<":" << (getCellLinearIndex(crtCell) % qcore::BOARD_SIZE + 1) <<  "]";

			if (endPoint == qcore::Direction::Up && parseVec[curentIndexInParseVec].first->up() && cellIsLastUpperRow(parseVec[curentIndexInParseVec].first->up()))
			{
				shortestPath.push_back({ (getCellLinearIndex(crtCell->up()) / qcore::BOARD_SIZE) , (getCellLinearIndex(crtCell->up()) % qcore::BOARD_SIZE)});
				break;// reached end
			}
			else if (endPoint == qcore::Direction::Down && parseVec[curentIndexInParseVec].first->down() && cellIsLastDownerRow(parseVec[curentIndexInParseVec].first->down()))
			{
				shortestPath.push_back({ (getCellLinearIndex(crtCell->down()) / qcore::BOARD_SIZE) , (getCellLinearIndex(crtCell->down()) % qcore::BOARD_SIZE)});
				break;// reached end
			}
			
			
			if (crtCell->up() && !visited[getCellLinearIndex(crtCell->up())] ) 
			{
				visited[getCellLinearIndex(crtCell->up())] = true;
				parseVec[parseVecSize++] = {crtCell->up(), curentIndexInParseVec};
			}
			if (crtCell->down() && !visited[getCellLinearIndex(crtCell->down())])
			{
				visited[getCellLinearIndex(crtCell->down())] = true;
				parseVec[parseVecSize++] = {crtCell->down(), curentIndexInParseVec};
			}				
			if (crtCell->left() && !visited[getCellLinearIndex(crtCell->left())])
			{
				visited[getCellLinearIndex(crtCell->left())] = true;
				parseVec[parseVecSize++] = {crtCell->left(), curentIndexInParseVec};
			}
			if (crtCell->right() && !visited[getCellLinearIndex(crtCell->right())])
			{
				visited[getCellLinearIndex(crtCell->right())] = true;
				parseVec[parseVecSize++] = {crtCell->right(), curentIndexInParseVec};
			}

			curentIndexInParseVec++;
			
		}

		// LOG_DEBUG(DOM) << "Max idx: " << curentIndexInParseVec;
		// std::string contents;
		// for (int i=0;i<=curentIndexInParseVec;i++)
		// {
		// 	Cell* crtCell = parseVec[i].first;
		// 	contents += "([" + std::to_string(getCellLinearIndex(crtCell) / qcore::BOARD_SIZE + 1)  + ":" + std::to_string(getCellLinearIndex(crtCell) % qcore::BOARD_SIZE + 1) +  "] " + std::to_string(parseVec[i].second) + "), ";
		// }

		// LOG_DEBUG(DOM) << "Parse vec, contents: " << contents;

		int parseIndex = curentIndexInParseVec;

		while(parseVec[parseIndex].second != -1)
		{
			Cell* &crtCell = parseVec[parseIndex].first;
			shortestPath.push_back({ (getCellLinearIndex(crtCell) / qcore::BOARD_SIZE) , (getCellLinearIndex(crtCell) % qcore::BOARD_SIZE)});

			parseIndex = parseVec[parseIndex].second;
		}
		
		//Cell* &crtCell = parseVec[parseIndex].first;
		//shortestPath.push_back({ (getCellLinearIndex(crtCell) / qcore::BOARD_SIZE) , (getCellLinearIndex(crtCell) % qcore::BOARD_SIZE)});

		// std::string logMes = "Shortest path from [" + std::to_string((int)pos.x + 1) + "-" + std::to_string( (int)pos.y + 1) + "]: ";

		// for (qcore::Position& step : shortestPath)
		//  	logMes += "[" +  std::to_string((int)step.x +1) + ":" +  std::to_string((int)step.y +1) + "] ";
		
		// LOG_INFO(DOM) << logMes;

		return shortestPath;
   }

   bool ABBoard::wallAllowed(qcore::Position pos, bool horizontal)
   {
		// return pos.x >0 && pos.x< qcore::BOARD_SIZE && pos.y >0 && pos.y< qcore::BOARD_SIZE && ((horizontal ? (getCell(pos.x, pos.y)->down() && getCell(pos.x, pos.y + 1)->down()) 
		// : (getCell(pos.x, pos.y)->right() && getCell(pos.x+1, pos.y)->right())) && getCell(pos.x, pos.y)->isWallStart() == false);
		return (horizontal ? (getCell(pos.x, pos.y)->down() && getCell(pos.x, pos.y + 1)->down()) 
		 : (getCell(pos.x, pos.y)->right() && getCell(pos.x+1, pos.y)->right())) && getCell(pos.x, pos.y)->isWallStart() == false;
   }

	 std::vector<qcore::WallState> ABBoard::getWalls()
	 {
		std::vector<qcore::WallState> wallCollection ;
		wallCollection.reserve(20);

		for(auto i=0;i<qcore::BOARD_SIZE;++i)
		{
			for(auto j=0;j<qcore::BOARD_SIZE;++j)
				if(getCell(i,j)->isWallStart())
				{
					if (not getCell(i,j)->down())
						wallCollection.push_back( {{i,j}, qcore::Orientation::Horizontal});
					else
					wallCollection.push_back( {{i,j}, qcore::Orientation::Vertical});
				}
		}

		//tmp:
		// std::string msg = "walls: ";
		// for (auto &it :wallCollection)
		// 	msg += "[" + std::to_string(it.position.x +1) + ":" + std::to_string(it.position.y +1) +"]"+ (it.orientation==qcore::Orientation::Horizontal ?"H" :"V") + " ";
		
		// printState();
		// LOG_INFO(DOM) << msg; 

		return wallCollection;
	 }
    
    
    
    
    
    
    
    
    ABBoardv2::ABBoardv2()
	{
		//LOG_INFO(DOM) << "ABBoard ver 2 constucted.";
	}

	//#define Pos2CellId(x,y) (y + x *qcore::BOARD_SIZE)
	#define cellIsLastUpperRowV2(x) (x<qcore::BOARD_SIZE)
	#define cellIsLastDownerRowV2(x) (x >= (qcore::BOARD_SIZE * (qcore::BOARD_SIZE - 1)))

	void ABBoardv2::placeValidWall(qcore::Position pos, bool horizontal)
	{
		//LOG_DEBUG(DOM) << "placing wall on corner: " << (int)pos.x +1<<":"<<(int)pos.y+1 << " o: " << horizontal;
		if (horizontal)
		{
			m_horizontalWalls.push_back(Pos2CellId(pos.x, pos.y));
		}
		else
		{
			m_VerticalWalls.push_back(Pos2CellId(pos.x, pos.y));
		}
	}



	std::vector<qcore::Position> ABBoardv2::shortestPath(qcore::Position pos, qcore::Direction endPoint)
	{
		//LOG_DEBUG(DOM) << "V2.Enter shortest path" ;

		std::array<bool, qcore::BOARD_SIZE*qcore::BOARD_SIZE> visited = {false}; // visited hash
		std::array<std::pair<uint8_t, short>, qcore::BOARD_SIZE*qcore::BOARD_SIZE>  parseVec;
		int parseVecSize = 1;
		
		parseVec[0] = { Pos2CellId(pos.x, pos.y),0 };
		visited[Pos2CellId(pos.x, pos.y)] = true;
		int curentIndexInParseVec = 0;

		while(curentIndexInParseVec < parseVecSize)
		{
			uint8_t &crtCell = parseVec[curentIndexInParseVec].first;
			
			// dbg only:
			//LOG_DEBUG(DOM) << "V2.Current cell: " <<  (int)crtCell << " [" << (int)(crtCell / qcore::BOARD_SIZE + 1)  <<":" << (int)(crtCell % qcore::BOARD_SIZE + 1) <<  "]";

			 if ((endPoint == qcore::Direction::Up && cellIsLastUpperRowV2(parseVec[curentIndexInParseVec].first) ) || // split iinto 2 functions for faster speed ??
			 	(endPoint == qcore::Direction::Down && cellIsLastDownerRowV2(parseVec[curentIndexInParseVec].first)))
			{
				//LOG_DEBUG(DOM) << "V2.Reached end with : " <<(int)crtCell;
				// reached end
				break;
			}
			
			
			if (!visited[upV2(crtCell)] && crtCell >= qcore::BOARD_SIZE &&  std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),upV2(crtCell)) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(upV2(crtCell))) == m_horizontalWalls.end())
			{
				//LOG_DEBUG(DOM) << "V2. Adding up : " << (int)upV2(crtCell);
				visited[upV2(crtCell)] = true;
				parseVec[parseVecSize++] = {upV2(crtCell), curentIndexInParseVec};
			}
			if (!visited[downV2(crtCell)] && crtCell < (qcore::BOARD_SIZE * (qcore::BOARD_SIZE-1)) && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),crtCell) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(crtCell)) == m_horizontalWalls.end() )
			{
				//LOG_DEBUG(DOM) << "V2. Adding down : " << (int)downV2(crtCell);
				visited[downV2(crtCell)] = true;
				parseVec[parseVecSize++] = {downV2(crtCell), curentIndexInParseVec};
			}
			if (!visited[rightV2(crtCell)] && (crtCell % qcore::BOARD_SIZE ) != (qcore::BOARD_SIZE-1) && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),crtCell) == m_VerticalWalls.end() &&
						std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),upV2(crtCell)) == m_VerticalWalls.end())
			{
				//LOG_DEBUG(DOM) << "V2. Adding right : " << (int)rightV2(crtCell);
				visited[rightV2(crtCell)] = true;
				parseVec[parseVecSize++] = {rightV2(crtCell), curentIndexInParseVec};
			}
			if (!visited[leftV2(crtCell)] && (crtCell % qcore::BOARD_SIZE) != 0 && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(crtCell)) == m_VerticalWalls.end() &&
				std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(upV2(crtCell))) == m_VerticalWalls.end())
			{
				//LOG_DEBUG(DOM) << "V2. Adding left : " << (int)leftV2(crtCell);
				visited[leftV2(crtCell)] = true;
				parseVec[parseVecSize++] = {leftV2(crtCell), curentIndexInParseVec};
			}

			curentIndexInParseVec++;
			
		}
		//LOG_DEBUG(DOM) << "V2.Max idx: " << curentIndexInParseVec;

		std::vector<qcore::Position> shortestPath;
		int parseIndex = curentIndexInParseVec;

		while(parseVec[parseIndex].second !=0)
		{
			uint8_t &crtCell = parseVec[parseIndex].first;
			shortestPath.push_back({ (crtCell / qcore::BOARD_SIZE) , (crtCell % qcore::BOARD_SIZE)});

			parseIndex = parseVec[parseIndex].second;
		}
		uint8_t &crtCell = parseVec[parseIndex].first;
		shortestPath.push_back({ (crtCell / qcore::BOARD_SIZE) , (crtCell % qcore::BOARD_SIZE)});


		// std::string logMes = "V2.Shortest path from [" + std::to_string((int)pos.x + 1) + "-" + std::to_string( (int)pos.y + 1) + "]: ";

		// for (qcore::Position& step : shortestPath)
		// 	logMes += "[" +  std::to_string((int)step.x +1) + ":" +  std::to_string((int)step.y +1) + "] ";
		
		// LOG_INFO(DOM) << logMes;

		return shortestPath;
	}

	int ABBoardv2::shortestPathLen(qcore::Position pos, qcore::Direction endPoint)
	{
		if ((endPoint == qcore::Direction::Up && cellIsLastUpperRowV2(Pos2CellId(pos.x, pos.y))) ||
			(endPoint == qcore::Direction::Down && cellIsLastDownerRowV2(Pos2CellId(pos.x, pos.y))))
		{
			return -100;
		}
			

		std::array<bool, qcore::BOARD_SIZE*qcore::BOARD_SIZE> visited = {false}; // visited hash
		std::array<uint8_t, qcore::BOARD_SIZE*qcore::BOARD_SIZE>  parseVec;
		int parseVecSize = 1;
		
		parseVec[0] = Pos2CellId(pos.x, pos.y);
		visited[Pos2CellId(pos.x, pos.y)] = true;
		int currentDistance = 0;
		int nextDistanceLevelSwitch = 0;
		int curentIndexInParseVec = 0;

		while(curentIndexInParseVec < parseVecSize)
		{
			uint8_t &crtCell = parseVec[curentIndexInParseVec];
			
			// dbg only:
			//LOG_DEBUG(DOM) << "Current cell: [" << (getCellLinearIndex(crtCell) / qcore::BOARD_SIZE + 1)  <<":" << (getCellLinearIndex(crtCell) % qcore::BOARD_SIZE + 1) <<  "]";

			 if ((endPoint == qcore::Direction::Up && cellIsLastUpperRowV2(parseVec[curentIndexInParseVec]) ) || // split iinto 2 functions for faster speed ??
			 	(endPoint == qcore::Direction::Down && cellIsLastDownerRowV2(parseVec[curentIndexInParseVec])))
				{
					// reached end
					//LOG_DEBUG(DOM) << "reached end. distance : " << currentDistance;
					//break;
					//LOG_INFO(DOM) << "V2.Shortest path from [" << pos.x + 1 << ":" << pos.y + 1 <<" ]: "<< currentDistance;
					return currentDistance;
				}
			
			//if (crtCell->up() && !visited[getCellLinearIndex(crtCell->up())] ) // split optimization...
			if (!visited[upV2(crtCell)] && crtCell >=qcore::BOARD_SIZE && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),upV2(crtCell)) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(upV2(crtCell))) == m_horizontalWalls.end())
			{
				visited[upV2(crtCell)] = true;
				parseVec[parseVecSize++] = upV2(crtCell);
			}
				
			if (!visited[downV2(crtCell)] && crtCell < (qcore::BOARD_SIZE * (qcore::BOARD_SIZE-1)) && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),crtCell) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(crtCell)) == m_horizontalWalls.end() )
			{
				visited[downV2(crtCell)] = true;
				parseVec[parseVecSize++] = downV2(crtCell);
			}
				
			if (!visited[rightV2(crtCell)] && (crtCell % qcore::BOARD_SIZE ) != (qcore::BOARD_SIZE-1) && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),crtCell) == m_VerticalWalls.end() &&
						std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),upV2(crtCell)) == m_VerticalWalls.end())
			{
				visited[rightV2(crtCell)] = true;
				parseVec[parseVecSize++] = rightV2(crtCell);
			}
				
			if (!visited[leftV2(crtCell)] && (crtCell % qcore::BOARD_SIZE) != 0 && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(crtCell)) == m_VerticalWalls.end() &&
				std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(upV2(crtCell))) == m_VerticalWalls.end())
			{
				visited[leftV2(crtCell)] = true;
				parseVec[parseVecSize++] = leftV2(crtCell);
			}

			if (curentIndexInParseVec == nextDistanceLevelSwitch)
			{
				currentDistance++;
				nextDistanceLevelSwitch = parseVecSize - 1;
				//LOG_DEBUG(DOM) << "Distance increased at: " << currentDistance << " next switch : " << nextDistanceLevelSwitch;
			}
				
			curentIndexInParseVec++;
			//LOG_DEBUG(DOM) << "moving to next idx: " << curentIndexInParseVec;
		}

		//LOG_INFO(DOM) << "V2.Shortest path from [" << pos.x + 1 << ":" << pos.y + 1 <<" ]: "<< - 1;
		return -1;
	}

	bool ABBoardv2::wallAllowed(qcore::Position pos, bool horizontal)
	{
		if (horizontal)
		{
			return std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),Pos2CellId(pos.x, pos.y)) == m_VerticalWalls.end() && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),Pos2CellId(pos.x, pos.y)) == m_horizontalWalls.end() &&
				std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),Pos2CellId(pos.x, pos.y + 1)) == m_horizontalWalls.end();
		}
		return std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),Pos2CellId(pos.x, pos.y)) == m_horizontalWalls.end() && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),Pos2CellId(pos.x, pos.y)) == m_VerticalWalls.end() &&
				std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),Pos2CellId(pos.x + 1, pos.y)) == m_VerticalWalls.end();
	}


	bool ABBoardv2::moveAllowed(qcore::Position pos,  qcore::Direction direction)
	{  
		return 
			(direction == qcore::Direction::Up && Pos2CellId(pos.x, pos.y) >= qcore::BOARD_SIZE && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),upV2(Pos2CellId(pos.x, pos.y))) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(upV2(Pos2CellId(pos.x, pos.y)))) == m_horizontalWalls.end()) ||
			(direction == qcore::Direction::Right && ( Pos2CellId(pos.x, pos.y) % qcore::BOARD_SIZE ) != (qcore::BOARD_SIZE-1) && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(), Pos2CellId(pos.x, pos.y)) == m_VerticalWalls.end() &&
						std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),upV2( Pos2CellId(pos.x, pos.y))) == m_VerticalWalls.end()) ||
			(direction == qcore::Direction::Down && Pos2CellId(pos.x, pos.y) < (qcore::BOARD_SIZE * (qcore::BOARD_SIZE-1)) && std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),Pos2CellId(pos.x, pos.y)) == m_horizontalWalls.end() &&
						std::find(m_horizontalWalls.begin(), m_horizontalWalls.end(),leftV2(Pos2CellId(pos.x, pos.y))) == m_horizontalWalls.end()  ) ||
			(direction == qcore::Direction::Left && (Pos2CellId(pos.x, pos.y) % qcore::BOARD_SIZE) != 0 && std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(Pos2CellId(pos.x, pos.y))) == m_VerticalWalls.end() &&
				std::find(m_VerticalWalls.begin(), m_VerticalWalls.end(),leftV2(upV2(Pos2CellId(pos.x, pos.y)))) == m_VerticalWalls.end());
	}

	void ABBoardv2::printState()
   {
       LOG_DEBUG(DOM)<<"Start print";
       std::stringstream ss;

	   ss <<"\n\n";
       for (int i=0;i<qcore::BOARD_SIZE;i++)
		{
			for (int j=0;j<qcore::BOARD_SIZE;j++)
			{
				ss << "[" << (i+1) << ":" << (j+1)<< "]";
				if ( moveAllowed({i,j}, qcore::Direction::Right))
					ss<< std::setw(3) <<" ";
				else 
					ss <<  std::setw(3) <<"|";
			}
			ss<<"\n";
			for (int j=0;j<qcore::BOARD_SIZE;j++)
			{
				if ( not moveAllowed({i,j}, qcore::Direction::Down))
				{
					if (not moveAllowed({i,j}, qcore::Direction::Right))
					{
						ss<< std::setw(8) << "-------|";
					}
					else
						ss<< std::setw(8) << "--------";
				}
				else if ( not moveAllowed({i,j}, qcore::Direction::Right))
					ss<< std::setw(8) << "|";
				else
					ss<< std::setw(8) <<" ";
			}
			ss<<"\n";
		}
		LOG_DEBUG(DOM) << ss.str();
   }
}

