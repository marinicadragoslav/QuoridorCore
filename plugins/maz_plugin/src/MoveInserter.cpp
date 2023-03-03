#include <algorithm>
#include <cmath>
#include "SolutionNode.h"
#include "MoveInserter.h"

namespace qplugin
{
    double dist(Position a, Position b)
    {
        return sqrt(pow(a.x - b.x, 2) +
                pow(a.y - b.y, 2));
    }

      std::set<Move> EverythingInserter::getMoves(ABBoardCaseNode* crtBoardState, bool self)
      {
            std::set<Move> moveSet;
            //moveSet.reserve(135);

            Position pos = crtBoardState->getPlayerPos(self);

            //LOG_INFO(DOM) << "My pos: " << (int)pos.x + 1 << ":" << (int)pos.y + 1;
             
            if((pos.x -1) >=0) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x - 1, pos.y}, false   }  )); // up
            if((pos.y +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y + 1}, false   }  )); // right
            if((pos.x +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x + 1, pos.y}, false   }  )); // down
            if((pos.y -1) >= 0)  moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y - 1}, false   }  )); // left

            //moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, INVALID_POS, false   }  )); // follow path


            if (crtBoardState->hasWallsLeft(self))
            {
                for (int8_t i=0;i<(qcore::BOARD_SIZE-1);i++ )
                {
                    for (int8_t j=0;j<(qcore::BOARD_SIZE-1);j++)
                    {
                        moveSet.emplace(std::forward<Move>({ {{i,j}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        moveSet.emplace(std::forward<Move>({ {{i,j}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                    }
                }
            }

            //Position oponent = crtBoardState->getPlayerPos(!self);

            //std::sort(moveSet.begin(), moveSet.end(),  [oponent](const auto& lhs, const auto& rhs){ return dist(oponent, lhs.pos) < dist(oponent, rhs.pos); });

            //LOG_INFO(DOM) << "EverythingInserter get Moves exit. " <<moveSet.size() << " moves";

            return moveSet;
      }

      

      std::set<Move> ExtendedHeuristicInserter::getMoves(ABBoardCaseNode* crtBoardState, bool self)
      {
            #define IF_POS_ADD_H(pos) {Position nwP = pos; if(nwP.x >=0 && nwP.x < (qcore::BOARD_SIZE-1) && nwP.y >=0 && nwP.y< (qcore::BOARD_SIZE-1)) \
                                moveSet.emplace(std::forward<Move>({ {{nwP.x,nwP.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));}
            #define IF_POS_ADD_V(pos) {Position nwP = pos; if(nwP.x >= 0 && nwP.x < (qcore::BOARD_SIZE-1) && nwP.y >= 0 && nwP.y< (qcore::BOARD_SIZE-1)) \
                                moveSet.emplace(std::forward<Move>({ {{nwP.x,nwP.y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));}

            std::set<Move> moveSet;
            //moveSet.reserve(100);

            Position pos = crtBoardState->getPlayerPos(self);

            if((pos.x -1) >=0) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x - 1, pos.y}, false   }  )); // up
            if((pos.y +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y + 1}, false   }  )); // right
            if((pos.x +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x + 1, pos.y}, false   }  )); // down
            if((pos.y -1) >= 0)  moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y - 1}, false   }  )); // left


            //moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, INVALID_POS, false   }  )); // follow path

            if (crtBoardState->hasWallsLeft(self))
            {
                using namespace qcore::literals;

                //Soround oponent:
                Position pos = crtBoardState->getPlayerPos(!self);
                IF_POS_ADD_H(pos);
                IF_POS_ADD_H(pos - 1_y);
                IF_POS_ADD_H(pos - 1_x);
                IF_POS_ADD_H(pos - 1_x - 1_y);
                IF_POS_ADD_V(pos -1_x);
                IF_POS_ADD_V(pos);
                IF_POS_ADD_V(pos -1_x - 1_y);
                IF_POS_ADD_V(pos -1_y);
                

                pos = crtBoardState->getPlayerPos(self);
                
                if((pos.x -1) >=0) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x - 1, pos.y}, false   }  )); // up
                if((pos.y +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y + 1}, false   }  )); // right
                if((pos.x +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x + 1, pos.y}, false   }  )); // down
                if((pos.y -1) >= 0)  moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y - 1}, false   }  )); // left

                IF_POS_ADD_H(pos);
                IF_POS_ADD_H(pos - 1_y);
                IF_POS_ADD_H(pos - 1_x);
                IF_POS_ADD_H(pos - 1_x - 1_y);
                IF_POS_ADD_V(pos -1_x);
                IF_POS_ADD_V(pos);
                IF_POS_ADD_V(pos -1_x - 1_y);
                IF_POS_ADD_V(pos -1_y);


                auto walls = crtBoardState->getCurrentWalls();

                for (auto &wall : walls)
                {
                    if (wall.orientation == qcore::Orientation::Horizontal)
                    {
                        IF_POS_ADD_V(wall.position - 1_x - 1_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x-1,pos.y-1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        IF_POS_ADD_H(wall.position - 2_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x,pos.y-2}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position - 1_y); // moveSet.emplace(std::forward<Move>({ {{wall.position.x,pos.y-1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position - 1_y + 1_x);// moveSet.emplace(std::forward<Move>({ {{wall.position.x+1,pos.y-1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));

                        IF_POS_ADD_V(wall.position - 1_x + 1_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x-1,pos.y+1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position + 1_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x,pos.y+1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position + 1_x + 1_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x+1,pos.y+1}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position + 2_y);// moveSet.emplace(std::forward<Move>({ {{wall.position.x+2,pos.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));

                        IF_POS_ADD_V(wall.position - 1_x);
                        IF_POS_ADD_V(wall.position + 1_x);
                    }
                    else
                    {
                        IF_POS_ADD_H(wall.position -1_x - 1_y);//moveSet.emplace(std::forward<Move>({ {{wall.position.x-1,pos.y-1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_H(wall.position -1_x);//moveSet.emplace(std::forward<Move>({ {{wall.position.x-1,pos.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_H(wall.position -1_x + 1_y);//moveSet.emplace(std::forward<Move>({ {{wall.position.x-1,pos.y+1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position - 2_x);//moveSet.emplace(std::forward<Move>({ {{wall.position.x-2,pos.y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));

                        IF_POS_ADD_H(wall.position +1_x - 1_y);//moveSet.emplace(std::forward<Move>({ {{wall.position.x+1,pos.y-1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_H(wall.position +1_x);//moveSet.emplace(std::forward<Move>({ {{wall.position.x+1,pos.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_H(wall.position +1_x + 1_y);//moveSet.emplace(std::forward<Move>({ {{wall.position.x+1,pos.y+1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                        IF_POS_ADD_V(wall.position + 2_x);//moveSet.emplace(std::forward<Move>({ {{wall.position.x+2,pos.y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));

                        IF_POS_ADD_H(wall.position - 1_y);
                        IF_POS_ADD_H(wall.position + 1_y);
                    }
                }
            }

            //todo: remove dupli-cats
            //std::sort( moveSet.begin(), moveSet.end() );
            //moveSet.erase( std::unique( moveSet.begin(), moveSet.end() ), moveSet.end() );
            
            //LOG_INFO(DOM) << "ExtendedHeuristicInserter get Moves exit. " <<moveSet.size() << " moves";

            return moveSet;
      }

      
      std::set<Move> ShortestPathInserter::getMoves(ABBoardCaseNode* crtBoardState, bool self)
      {
            #define IF_POS_ADD_H(pos) {Position nwP = pos; if(nwP.x >=0 && nwP.x < (qcore::BOARD_SIZE-1) && nwP.y >=0 && nwP.y< (qcore::BOARD_SIZE-1)) \
                                moveSet.emplace(std::forward<Move>({ {{nwP.x,nwP.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));}
            #define IF_POS_ADD_V(pos) {Position nwP = pos; if(nwP.x >= 0 && nwP.x < (qcore::BOARD_SIZE-1) && nwP.y >= 0 && nwP.y< (qcore::BOARD_SIZE-1)) \
                                moveSet.emplace(std::forward<Move>({ {{nwP.x,nwP.y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));}


            std::set<Move> moveSet;
            //moveSet.reserve(50);

            Position pos = crtBoardState->getPlayerPos(self);

            if((pos.x -1) >=0) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x - 1, pos.y}, false   }  )); // up
            if((pos.y +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y + 1}, false   }  )); // right
            if((pos.x +1) <  qcore::BOARD_SIZE) moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x + 1, pos.y}, false   }  )); // down
            if((pos.y -1) >= 0)  moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, {pos.x, pos.y - 1}, false   }  )); // left

            //moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, INVALID_POS, false   }  )); // follow path  TODO: remove call . implement jumps in normal calls


            if (crtBoardState->hasWallsLeft(self))
            {
                using namespace qcore::literals;
                auto sp = crtBoardState->getShortestPath(!self);
                sp.push_back(crtBoardState->getPlayerPos(!self));

                auto &fp = sp.front();
                moveSet.emplace(std::forward<Move>({ {{fp.x,fp.y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                //LOG_INFO(DOM) << "Added Horizontal wall in on :" << (int)fp.x+1 << ":" << (int)fp.y+1;

                for (int i=0;i<sp.size()-1;i++)
                {
                    if (sp[i].x == sp[i+1].x)
                    {
                        // vertical move
                        if (sp[i].y < sp[i+1].y)
                        {
                            //moveSet.emplace(std::forward<Move>({ {{sp[i].x,sp[i].y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                            IF_POS_ADD_V(sp[i]);
                            //LOG_INFO(DOM) << "Added vertical wall in on :" << (int)sp[i].x+1 << ":" << (int)sp[i].y+1;
                            //moveSet.emplace(std::forward<Move>({ {{sp[i].x-1,sp[i].y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                            IF_POS_ADD_V(sp[i] - 1_x);
                        }
                        else
                        {
                            //moveSet.emplace(std::forward<Move>({ {{sp[i+1].x,sp[i+1].y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                            IF_POS_ADD_V(sp[i+1]);
                            //LOG_INFO(DOM) << "Added vertical wall in on :" << (int)sp[i+1].x+1 << ":" << (int)sp[i+1].y+1;
                            //moveSet.emplace(std::forward<Move>({ {{sp[i].x-1,sp[i].y}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                            IF_POS_ADD_V(sp[i+1] - 1_x);
                        }
                        
                    }
                    else
                    {
                        if (sp[i].x > sp[i+1].x)
                        {
                            //moveSet.emplace(std::forward<Move>({ {{sp[i+1].x,sp[i+1].y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                            IF_POS_ADD_H(sp[i+1]);
                            //LOG_INFO(DOM) << "Added Horizontal wall in on :" << (int)sp[i+1].x+1 << ":" << (int)sp[i].y+1;

                            //moveSet.emplace(std::forward<Move>({ {{sp[i+1].x,sp[i+1].y-1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                            IF_POS_ADD_H(sp[i+1] - 1_y);
                        }
                        else
                        {
                            //moveSet.emplace(std::forward<Move>({ {{sp[i].x,sp[i].y}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                            IF_POS_ADD_H(sp[i]);
                            //LOG_INFO(DOM) << "Added Horizontal wall in on :" << (int)sp[i].x+1 << ":" << (int)sp[i].y+1;

                            //moveSet.emplace(std::forward<Move>({ {{sp[i].x,sp[i].y-1}, qcore::Orientation::Horizontal}, INVALID_POS, true   }  ));
                            IF_POS_ADD_H(sp[i] - 1_y) ;
                        }
                        
                    }
                }
            }

             
            //LOG_INFO(DOM) << "ShortestPathInserter get Moves exit. " <<moveSet.size() << " moves";

            return moveSet;
      }
      

      std::set<Move> SchilerInserter::getMoves(ABBoardCaseNode* crtBoardState, bool self)
      {
            LOG_INFO(DOM) << "SchilerInserter get Moves enter-";
            std::set<Move> moveSet;

            m_callCount++;

            if (m_callCount < 3)
            {
                moveSet.emplace(std::forward<Move>({ {{0,0}, qcore::Orientation::Horizontal}, INVALID_POS, false   }  )); // follow path
            }
            else
            {
                srand(time(NULL));
                
                int8_t i,j;
                do
                {
                    i = 7;
                    //i = 0;//4 + (rand() % 4);
                    j = 3 + (rand() % (qcore::BOARD_SIZE - 6));

                    if (crtBoardState->wallAllowed({{0,0}, qcore::Orientation::Vertical}))
                    {
                        moveSet.emplace(std::forward<Move>({ {{i,j}, qcore::Orientation::Vertical}, INVALID_POS, true   }  ));
                        break;
                    }
                } while (true);
            }
            
            return moveSet;
      }
      
}