/*
 * BPlayer.cpp
 *
 *  Created on: Nov 16, 2022
 *      Author: uib15381
 */

#include "BPlayer.h"
#include "QcoreUtil.h"
#include "BoardState.h"
#include <queue>
#include "PlayerAction.h"
#include <algorithm>
#include <limits>
#include <type_traits>
#include "Game.h"
#include <stdio.h>
#include <thread>
#include <future>
#include <chrono>
#include "QcoreUtil.h"

using namespace qcore::literals;
using namespace qcore;

namespace
{
    /** Log domain */
    const char * const DOM = "qplugin::BUR";
    std::list<qcore::Position> pos ,ad_pos;
    qcore::Position empty_pos = qcore::Position(0,0);
    qcore::Position initial_pos = qcore::Position(1,1);
    bool problem = false;
    std::chrono::time_point<std::chrono::system_clock> start_time, end_time;
}

void PrintAsciiGameBoard(qcore::BoardMap &map)
{
#ifdef WIN32
    const std::string TABLE_VERTICAL_BORDER = std::string(1, static_cast<char>(186));
    const std::string TABLE_HORIZONTAL_BORDER = std::string(1, static_cast<char>(205));
    const std::string TABLE_BOTTOM_LEFT_BORDER = std::string(1, static_cast<char>(200));
    const std::string TABLE_BOTTOM_RIGHT_BORDER = std::string(1, static_cast<char>(188));
    const std::string TABLE_TOP_LEFT_BORDER = std::string(1, static_cast<char>(201));
    const std::string TABLE_TOP_RIGHT_BORDER = std::string(1, static_cast<char>(187));
    const std::string QUORIDOR_HORIZONTAL_WALL = std::string(3, static_cast<char>(196));
    const std::string QUORIDOR_VERTICAL_WALL = std::string(1, static_cast<char>(179));
#else
    const std::string TABLE_VERTICAL_BORDER = std::string("\u2551");
    const std::string TABLE_HORIZONTAL_BORDER = std::string("\u2550");
    const std::string TABLE_BOTTOM_LEFT_BORDER = std::string("\u255A");
    const std::string TABLE_BOTTOM_RIGHT_BORDER = std::string("\u255D");
    const std::string TABLE_TOP_LEFT_BORDER = std::string("\u2554");
    const std::string TABLE_TOP_RIGHT_BORDER = std::string("\u2557");
    const std::string QUORIDOR_HORIZONTAL_WALL = std::string("\u2500\u2500\u2500");
    const std::string QUORIDOR_VERTICAL_WALL = std::string("\u2502");
#endif

    const std::string TABLE_TOP_MARGIN = "\n     ";
    const std::string TABLE_BOTTOM_MARGIN = "\n";
    const std::string TABLE_LEFT_MARGIN = "   ";
    const std::string TABLE_RIGHT_MARGIN = " ";
    const int TABLE_LEFT_INDEX_WIDTH = 2;
    const std::string TABLE_LEFT_PADDING = " ";
    const std::string TABLE_RIGHT_PADDING = " ";

   qcore::BoardMap coloredMap;

   system("clear");

   std::cout << TABLE_TOP_MARGIN;

   for (int i = 0; i < qcore::BOARD_SIZE; ++i)
   {
      std::cout << std::setfill(' ') << std::setw(6) << i;
   }

   std::cout <<  "\n";
   std::cout << TABLE_LEFT_MARGIN << "0 " << TABLE_TOP_LEFT_BORDER;

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << TABLE_HORIZONTAL_BORDER;
   }

   std::cout << TABLE_TOP_RIGHT_BORDER << TABLE_RIGHT_MARGIN << "\n";

   for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
   {
      std::cout << TABLE_LEFT_MARGIN;

      if (i & 1)
      {
         std::cout << std::setw(TABLE_LEFT_INDEX_WIDTH) << std::left << (i / 2) + 1 << TABLE_VERTICAL_BORDER;
      }
      else
      {
         std::cout << std::string(TABLE_LEFT_INDEX_WIDTH, ' ') << TABLE_VERTICAL_BORDER;
      }

      std::cout << TABLE_LEFT_PADDING;

      for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
      {
         switch (map(i, j))
         {
            case 0:
               std::cout << "   ";
               break;
            case qcore::BoardMap::VertivalWall:
               std::cout << " " << QUORIDOR_VERTICAL_WALL << " ";
               break;
            case qcore::BoardMap::HorizontalWall:
               std::cout << QUORIDOR_HORIZONTAL_WALL;
               break;
            case qcore::BoardMap::Pawn0:
               std::cout << " X ";
               break;
            case qcore::BoardMap::Pawn1:
               std::cout << " 1 ";
               break;
            case qcore::BoardMap::Pawn2:
               std::cout << " 2 ";
               break;
            case qcore::BoardMap::Pawn3:
               std::cout << " 3 ";
               break;
            default:
               std::cout << " " << map(i, j) << " ";
               break;
         }
      }

      std::cout << TABLE_RIGHT_PADDING << TABLE_VERTICAL_BORDER  << TABLE_RIGHT_MARGIN << "\n";
   }

   std::cout << TABLE_LEFT_MARGIN << std::string(TABLE_LEFT_INDEX_WIDTH, ' ') << TABLE_BOTTOM_LEFT_BORDER;

   for (int i = 0; i < qcore::BOARD_SIZE * 6 - 1; ++i)
   {
      std::cout << TABLE_HORIZONTAL_BORDER;
   }

   std::cout << TABLE_BOTTOM_RIGHT_BORDER << TABLE_RIGHT_MARGIN << "\n" << TABLE_BOTTOM_MARGIN;
}

void printBoard(std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> &pred)
{
    for (int i = 0; i < qcore::BOARD_MAP_SIZE; ++i)
    {
        for (int j = 0; j < qcore::BOARD_MAP_SIZE; ++j)
        {
            std::cout << "(" <<  static_cast<int>(pred[i][j].x) << "," << static_cast<int>(pred[i][j].y) << ")";

        }
        std::cout << std::endl;
    }
    std::cout << std::endl;
}

std::list<qcore::Position> create_path_and_move(qcore::BoardMap &map, qcore::Position& myPos, std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> &pred)
{
    bool stop_flag = false;
    std::list<qcore::Position> adiacent_pos;
    std::list<qcore::Position> pos;

    pred[myPos.x * 2][myPos.y * 2] = initial_pos;

    auto create_viz_matrix = [&](const qcore::Position& p, qcore::Direction dir) -> bool
    {
        bool stop = false;

        qcore::Position new_position = p + dir;
        qcore::Position double_position = new_position + dir;

        if(new_position.x < qcore::BOARD_MAP_SIZE &&  new_position.x >= 0 &&
           new_position.y < qcore::BOARD_MAP_SIZE && new_position.y >= 0)
        {
            /* if not already visited and next square to the given direction is not a wall */
            if(map(new_position.x,new_position.y) != qcore::BoardMap::HorizontalWall &&
               map(new_position.x,new_position.y) != qcore::BoardMap::VertivalWall && pred[double_position.x][double_position.y] == empty_pos)
            {
                pred[double_position.x][double_position.y] = p;

                /* update list with adjacent neighbor */
                adiacent_pos.push_front(double_position);

                int check = 0;//((myPos.x - 6) *2 >= 0) ? (myPos.x - 6)*2 : 0;
                if(double_position.x == check) //TODO: If check remains 0, you can remove this variable
                {
                    stop = true;
                }
            }
        }
        return stop;
    };

    while(!stop_flag)
    {
        pos.clear();
        bool ret = false;

        if(!adiacent_pos.empty())
        {
            std::copy(adiacent_pos.begin(), adiacent_pos.end(),
                    std::back_insert_iterator<std::list<qcore::Position> >(pos));
            adiacent_pos.clear();
        }

        if (pos.empty())
        {
            for(int i = 0; i <= (int)qcore::Direction::Left; i++)
            {
                ret = create_viz_matrix(myPos * 2, qcore::Direction(i));
                if(ret) {break;}
            }
            if(ret)
            {
                stop_flag = true;
                break;
            }
            if(adiacent_pos.empty())
            {
                problem = true;
                stop_flag = true;
                break;
            }
        }
        else
        {
            for (auto const& n : pos)
            {
                // check all the variants
                for(int i = 0;i <= (int)qcore::Direction::Left; i++)
                {
                    ret = create_viz_matrix(n, qcore::Direction(i));
                    if(ret) {break;}
                }
                if(ret)
                {
                    stop_flag = true;
                    break;
                }
            }
        }
    }
    /* clear qcore::Position list here */
    int check = 0;//((myPos.x - 6) *2 >= 0) ? (myPos.x - 6)*2 : 0;
    pos.clear();

    for (int j = 0; j < qcore::BOARD_MAP_SIZE; j=j+2)
    {
        if (pred[check][j].x != 0 || pred[check][j].y != 0)
        {
            pos.push_front(qcore::Position(pred[check][j].x, pred[check][j].y));
            int8_t k = pred[check][j].x;
            int8_t l = pred[check][j].y;
            int8_t aux = 0;

            while (!(pred[k][l] == pred[myPos.x * 2][myPos.y * 2]))
            {
                pos.push_front(pred[k][l]);
                aux = pred[k][l].x;
                l = pred[k][l].y;
                k = aux;
            }

            pos.push_back(qcore::Position(check,j));
            break;
        }
    }

    return pos;
}

namespace qplugin
{

    /** Get player states from the player's perspective */
    std::vector<PlayerState> MyBoardState::getPlayers(const PlayerId id) const
    {
        std::vector<PlayerState> players;
        uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

        for ( auto& p : mPlayers )
        {
            players.push_back(p.rotate(rotations));
        }

        return players;
    }

    /** Get wall states from the player's perspective */
    std::list<WallState> MyBoardState::getWalls(const PlayerId id) const
    {
       std::list<WallState> walls;
       uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

       for ( auto& w : mWalls )
       {
          walls.push_back(w.rotate(rotations));
       }

       return walls;
    }

    /** Sets the specified action on the board, after it has been validated */
    void MyBoardState::applyAction(const PlayerAction& action)
    {
       PlayerState &player = mPlayers.at(action.playerId);
       mLastAction = action.rotate(4 - static_cast<int>(player.initialState));

       switch (action.actionType)
       {
          case ActionType::Move:
          {
             player.position = mLastAction.playerPosition;
             break;
          }
          case ActionType::Wall:
          {
             if (player.wallsLeft)
             {
                --player.wallsLeft;
             }

             mWalls.push_back(mLastAction.wallState);

             break;
          }
          default:
             break;
       }
    }

    void MyBoardState::createBoardMap(BoardMap& map, const PlayerId id) const
    {
       for (int i = 1; i < BOARD_MAP_SIZE; i += 2)
       {
          for (int j = 1; j < BOARD_MAP_SIZE; j += 2)
          {
             map(i, j) = BoardMap::MidWall;
          }
       }

       auto walls = getWalls(id);
       auto players = getPlayers(id);

       for (auto& w : walls)
       {
          if (w.orientation == Orientation::Vertical)
          {
             Position p = w.position * 2 - 1_y;
             map(p) = map(p + 1_x) = map(p + 2_x) = BoardMap::VertivalWall;
          }
          else
          {
             Position p = w.position * 2 - 1_x;
             map(p) = map(p + 1_y) = map(p + 2_y) = BoardMap::HorizontalWall;
          }
       }

       for (size_t i = 0; i < players.size(); ++i)
       {
          map(players[i].position * 2) = BoardMap::Pawn0 - i;
       }
    }

    GameState::GameState(qcore::PlayerId myId, qcore::Position myPos, qcore::Position opPos, int score, MyBoardState boardstate, uint8_t depth):
                m_myId(myId),
                m_myPos(myPos),
                m_opPos(opPos),
                m_score(score),
                m_boardstate(boardstate),
                m_depth(depth)
    {}

    BPlayer::BPlayer(qcore::PlayerId id, const std::string& name, qcore::GamePtr game):
        qcore::Player(id, name, game)
    {}

    void BPlayer::decide_to_move(std::list<qcore::Position> &pos)
    {
        // shortest path list
        if(!pos.empty())
        {
            pos.pop_front();
            qcore::Position first = pos.front();

            if (!move(first.x/2, first.y/2))
            {
                pos.pop_front();
                qcore::Position first = pos.front();
                move(first.x/2, first.y/2);
            }
        }
        else
        {
            std::cout << "Something went wrong, empty queue." << std::endl;
        }
    }

    int BPlayer::minimax(GameState& state, int alpha, int beta, bool maximizingPlayer, PlayerActionRefactor &move_param)
    {
        try
        {
            if (state.m_depth >= 3)
            {
                return state.m_score;
            }

            end_time = std::chrono::system_clock::now();
            std::chrono::duration<double> elapsed_seconds = end_time - start_time;
            if(elapsed_seconds.count() > 4.95)
            {
                throw util::Exception("Time expired!!");
            }

            if (maximizingPlayer)
            {
                int maxScore = std::numeric_limits<int>::min();

                for (PlayerActionRefactor move : state.getPossibleMoves())
                {
                    int score;
                    GameState nextState = state.applyMove(move);

                    if(nextState.m_depth != 3)
                    {
                        nextState.m_depth++;
                    }
                    score = minimax(nextState, alpha, beta, false, move);

                    if (maxScore < score)
                    {
                        move_param = move;
                        maxScore = score;
                    }
                    alpha = std::max(alpha, score);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
                return maxScore;
            }
            else
            {
                int minScore = std::numeric_limits<int>::max();

                for (PlayerActionRefactor move : state.getPossibleMoves())
                {
                    GameState nextState = state.applyMove(move);
                    nextState.m_depth= 2;
                    int score = minimax(nextState, alpha, beta, true, move);
                    minScore = std::min(minScore, score);
                    beta = std::min(beta, score);
                    if (beta <= alpha)
                    {
                        break;
                    }
                }
                return minScore;
            }
        }
        catch (std::exception& e)
        {
            return 0;
        }
    }

    /* Check if player's action is valid */
    bool GameState::isActionValid(PlayerActionRefactor& actionRef, std::string& reason)
    {
       try
       {
          std::stringstream ss;

          if (not actionRef.action.isValid())
          {
             throw util::Exception("Invalid action structure");
          }

          BoardMap map;
          m_boardstate.createBoardMap(map, actionRef.action.playerId);

          if (actionRef.action.actionType == ActionType::Move)
          {
             Position currentPos = m_boardstate.getPlayers(actionRef.action.playerId).at(actionRef.action.playerId).position;
             Position p1 = currentPos * 2;
             Position p2 = actionRef.action.playerPosition * 2;
             uint8_t dist = actionRef.action.playerPosition.dist(currentPos);

             if (map(p2) == BoardMap::Invalid)
             {
                throw util::Exception(ss.str());
             }

             if (p1 == p2)
             {
                throw util::Exception(ss.str());
             }

             if (map(p2))
             {
                throw util::Exception(ss.str());
             }

             if (dist == 1)
             {
                if (map((p1 + p2) / 2))
                {
                   throw util::Exception(ss.str());
                }
             }
             else if (dist == 2)
             {
                // Jump over another pawn
                if (p1.x == p2.x)
                {
                   int8_t mid = (p1.y + p2.y) / 2;

                   // There's no wall between
                   if (map(p1.x, mid - 1) or map(p1.x, mid + 1))
                   {
                      throw util::Exception(ss.str());
                   }

                   if (not map(p1.x, mid))
                   {
                      throw util::Exception(ss.str());
                   }
                }
                else if (p1.y == p2.y)
                {
                   int8_t mid = (p1.x + p2.x) / 2;

                   // There's no wall between
                   if (map(mid - 1, p1.y) or map(mid + 1, p1.y))
                   {
                      throw util::Exception(ss.str());
                   }

                   if (not map(mid, p1.y))
                   {
                      throw util::Exception(ss.str());
                   }
                }
                else
                {
                   // If there is a wall or a third pawn behind the second pawn, the player can place his pawn to the left or the right of the other pawn
                   Position mid1(p1.x, p2.y);
                   Position diff1 = p1 - mid1;

                   Position mid2(p2.x, p1.y);
                   Position diff2 = p1 - mid2;

                   if ((not map.isPawn(mid1) or map((p1 + mid1)/2) or map((p2 + mid1) / 2) or (map(mid1 + diff1 / 2) == 0 and not map.isPawn(mid1 + diff1))) and
                      (not map.isPawn(mid2) or map((p1 + mid2)/2) or map((p2 + mid2) / 2) or (map(mid2 + diff2 / 2) == 0 and not map.isPawn(mid2 + diff2))))
                   {
                      throw util::Exception(ss.str());
                   }
                }
             }
             else
             {
                throw util::Exception(ss.str());
             }
          }
          else
          {
             // Check number of walls left
             if (m_boardstate.getPlayers(actionRef.action.playerId).at(actionRef.action.playerId).wallsLeft + 2 == 0)
             {
                throw util::Exception(ss.str());
             }

             // Check board limits
             if (actionRef.action.wallState.position.x >= BOARD_SIZE or actionRef.action.wallState.position.y >= BOARD_SIZE or
                     actionRef.action.wallState.position.x < 0 or actionRef.action.wallState.position.y < 0 or
                (actionRef.action.wallState.position.x == 0 and actionRef.action.wallState.position.y == 0) or
                (actionRef.action.wallState.position.x == 0 and actionRef.action.wallState.orientation == Orientation::Horizontal) or
                (actionRef.action.wallState.position.y == 0 and actionRef.action.wallState.orientation == Orientation::Vertical) or
                (actionRef.action.wallState.position.x == BOARD_SIZE - 1 and actionRef.action.wallState.orientation == Orientation::Vertical) or
                (actionRef.action.wallState.position.y == BOARD_SIZE - 1 and actionRef.action.wallState.orientation == Orientation::Horizontal))
             {
                throw util::Exception(ss.str());
             }

             // Check if it is not intersecting other wall
             if (actionRef.action.wallState.orientation == Orientation::Vertical)
             {
                Position p = actionRef.action.wallState.position * 2 - 1_y;

                if (map(p) or map(p + 1_x) != BoardMap::MidWall or map(p + 2_x))
                {
                   throw util::Exception(ss.str());
                }

                map(p) = map(p + 1_x) = map(p + 2_x) = BoardMap::VertivalWall;
             }
             else
             {
                Position p = actionRef.action.wallState.position * 2 - 1_x;

                if (map(p) or map(p + 1_y) != BoardMap::MidWall or map(p + 2_y))
                {
                   throw util::Exception(ss.str());
                }

                map(p) = map(p + 1_y) = map(p + 2_y) = BoardMap::HorizontalWall;
             }

             BoardMap map_after_move_for_op;
             std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> ad_pos_pred;

             map_after_move_for_op = addWalltoBoardMap(actionRef.action.playerId^1, actionRef);
             ad_pos = create_path_and_move(map_after_move_for_op, m_opPos, ad_pos_pred);

             if(problem)
             {
                 problem = false;
                 throw util::Exception(ss.str());
             }
             else
             {
                 BoardMap map_after_move;
                 map_after_move = addWalltoBoardMap(actionRef.action.playerId, actionRef);
                 std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> my_pos_pred;

                 pos = create_path_and_move(map_after_move, m_myPos, my_pos_pred);
                 if(problem)
                 {
                     problem = false;
                     throw util::Exception(ss.str());
                 }
                 actionRef.m_mylen = pos.size();
                 actionRef.m_oplen = ad_pos.size();
             }

          }
       }
       catch (std::exception& e)
       {
          reason = e.what();
          return false;
       }

       return true;
    }

    /* Checks if the player's path isn't blocked */
    BoardMap GameState::addWalltoBoardMap(const PlayerId playerId, const PlayerActionRefactor& actionRef)
    {
        auto players = m_boardstate.getPlayers(playerId);
        auto currentPos = players.at(0).position * 2;

        BoardMap map;
        std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> my_pos_pred;

        m_boardstate.createBoardMap(map, playerId);
        map(currentPos) = BoardMap::Invalid;

        // Place the wall
        auto wall = actionRef.action.wallState.rotate(4 - static_cast<int>(players.at(actionRef.action.playerId).initialState));

        if (wall.orientation == Orientation::Vertical)
        {
            Position p = wall.position * 2 - 1_y;
            map(p) = map(p + 1_x) = map(p + 2_x) = BoardMap::VertivalWall;
        }
        else
        {
            Position p = wall.position * 2 - 1_x;
            map(p) = map(p + 1_y) = map(p + 2_y) = BoardMap::HorizontalWall;
        }

        return map;
    }

    vector <PlayerActionRefactor> GameState::getPossibleMoves()
    {
        vector<PlayerActionRefactor> player_move;
        std::string reason;

        if(m_depth == 2)
        {
            int index = 0, superior_index = BOARD_SIZE;

            if(m_myPos.rotate(2).x - 1 > 0)
            {
                index = m_myPos.rotate(2).x - 1 ;
            }
            if(m_myPos.rotate(2).x + 3 < superior_index)
            {
                superior_index = m_myPos.rotate(2).x + 3;
            }
            for (int i = superior_index; i > index; --i)
            {
                for (int j = superior_index; j > index; --j)
                {
                    WallState ws1 { Position(i, j), Orientation::Vertical };
                    WallState ws2 { Position(i, j), Orientation::Horizontal };

                    PlayerActionRefactor next_move;

                    next_move.action.playerId = m_myId;
                    next_move.action.wallState = ws1;
                    next_move.action.actionType = ActionType::Wall;

                    if (isActionValid(next_move, reason))
                    {
                        player_move.push_back(next_move);
                    }

                    next_move.action.wallState = ws2;
                    next_move.action.actionType = ActionType::Wall;

                    if (isActionValid(next_move, reason))
                    {
                        player_move.push_back(next_move);
                    }
                }
            }
        }
        else
        {
            for (int i = 0; i < BOARD_SIZE; ++i)
            {
                for (int j = 0; j < BOARD_SIZE; ++j)
                {
                    WallState ws1 { Position(i, j), Orientation::Vertical };
                    WallState ws2 { Position(i, j), Orientation::Horizontal };

                    PlayerActionRefactor next_move;

                    next_move.action.playerId = m_myId;
                    next_move.action.wallState = ws1;
                    next_move.action.actionType = ActionType::Wall;

                    if (isActionValid(next_move, reason))
                    {
                        player_move.push_back(next_move);
                    }

                    next_move.action.wallState = ws2;
                    next_move.action.actionType = ActionType::Wall;

                    if (isActionValid(next_move, reason))
                    {
                        player_move.push_back(next_move);
                    }
                }
            }
        }

        if(m_depth == 0)
        {
            PlayerActionRefactor next_move;
            next_move.action.playerPosition = m_myPos;
            //player_move.push_back(next_move);
            for (int i = 0; i <= (int)qcore::Direction::Left; i++)
            {

                next_move.action.playerId = m_myId;
                next_move.action.playerPosition = m_myPos + qcore::Direction(i);
                next_move.action.actionType = ActionType::Move;

                if(isActionValid(next_move, reason))
                {
                    player_move.push_back(next_move);
                }
            }
        }

        return player_move;
    }

    GameState GameState::applyMove(PlayerActionRefactor& player_decide_move)
    {
        GameState updated_game(m_myId,m_myPos, m_opPos, m_score, m_boardstate, m_depth);

        updated_game.m_boardstate.applyAction(player_decide_move.action);

        /* at every applied wall, we should calculate shortest path of each again, and calculate the score, max score for winning player */
        updated_game.m_myPos = updated_game.m_boardstate.getPlayers(m_myId).at(m_myId).position;
        updated_game.m_opPos = updated_game.m_boardstate.getPlayers(m_myId^1).at(m_myId^1).position;


        if (updated_game.m_opPos.x != 0)
        {
            updated_game.m_score = (player_decide_move.m_oplen - 1- player_decide_move.m_mylen) * (updated_game.m_opPos.x) ;
        }
        else
        {
            updated_game.m_score = (player_decide_move.m_oplen - 1 - player_decide_move.m_mylen);
        }

        return updated_game;
    }

    void BPlayer::doNextMove()
    {
        qcore::BoardMap map, ad_map;

        start_time = std::chrono::system_clock::now();

        LOG_INFO(DOM) << "Player " << (int)getId() << " is thinking ..";

        qcore::Position myPos = getBoardState()->getPlayers(getId()).at(getId()).position;
        qcore::Position adPos_rotate = getBoardState()->getPlayers(getId()).at(getId()^1).position.rotate(2);

        getBoardState()->createBoardMap(map, getId());
        getBoardState()->createBoardMap(ad_map, getId()^1);

        // initial phase

        std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> my_pos_pred;
        std::array<std::array<qcore::Position, qcore::BOARD_MAP_SIZE>, qcore::BOARD_MAP_SIZE> ad_pos_pred;

        pos = create_path_and_move(map, myPos, my_pos_pred);
        ad_pos = create_path_and_move(ad_map, adPos_rotate, ad_pos_pred);

        if(getBoardState()->getPlayers(0).at(getId()).wallsLeft == 0 || (ad_pos.size() -1 - pos.size() == 0 && (getBoardState()->getPlayers(0).at(getId()).wallsLeft == 10)))
        {
            decide_to_move(pos);
        }
        else
        {
            int alpha = std::numeric_limits<int>::min();
            int beta = std::numeric_limits<int>::max();
            PlayerActionRefactor gameState;
            uint8_t depth = 0;
            qcore::PlayerId myId = getId();

            BoardState state = *getBoardState();
            MyBoardState myState(state, myId);

            GameState currentState(myId, myPos, adPos_rotate, 0, myState, depth);

            /* Maximizing player called with parameter true */
            minimax(currentState, alpha, beta, true, gameState);

            if(gameState.action.actionType ==  qcore::ActionType::Move)
            {
                move(gameState.action.playerPosition);
            }
            else
            {
                placeWall(gameState.action.wallState);
            }
        }
    }

} //namespace qplugin

