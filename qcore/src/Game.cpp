#include "PlayerAction.h"
#include "GameServer.h"
#include "Game.h"
#include "QcoreUtil.h"

#include <cstring>
#include <sstream>

using namespace qcore::literals;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::QG";

   /** Construction */
   Game::Game(uint8_t players) :
      mNumberOfPlayers(players),
      mBoardState(std::make_shared<BoardState>(players)),
      mCurrentPlayer(0)
   {
   }

   void Game::setGameServer(std::shared_ptr<GameServer> gameServer)
   {
      std::lock_guard<std::mutex> lock(mMutex);
      mGameServer = gameServer;
   }

   /** Returns the ID of the player on move */
   PlayerId Game::getCurrentPlayer() const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      return mCurrentPlayer;
   }

   /** Waits until the specified player has picked his move */
   void Game::waitPlayerMove(PlayerId playerId)
   {
      std::unique_lock<std::mutex> lock(mMutex);

      while (mCurrentPlayer == playerId)
      {
         // TODO Set a timeout
         mCv.wait(lock);
      }
   }

   /** Validates and sets the next user action */
   bool Game::processPlayerAction(const PlayerAction& action, std::string& reason)
   {
      std::lock_guard<std::mutex> lock(mMutex);

      if (not isActionValid(action, reason))
      {
         // TODO: Keep some user statistics and kick player after a configurable number of
         // illegal moves.

         LOG_WARN(DOM) << reason;
         return false;
      }

#ifdef BOOST_AVAILABLE
      // Notify all remote boards of the state change
      if (mGameServer)
      {
         LOG_DEBUG(DOM) << "Notify remote players of state change ...";
         mGameServer->send(std::string{ GameServer::BoardStateUpdate } + action.serialize());
      }
#endif

      // Set the action
      setAction(action);

      // Update player's turn
      mCurrentPlayer = (mCurrentPlayer + 1) % mNumberOfPlayers;
      mCv.notify_all();

      return true;
   }

   /** Sets the specified action on the board, after it has been validated */
   void Game::setAction(const PlayerAction& action)
   {
      switch (action.actionType)
      {
         case ActionType::Move:
            mBoardState->setPlayerPosition(action.playerId, action.position);
            break;
         case ActionType::Wall:
            mBoardState->addWall(action.playerId, action.position, action.wallOrientation);
            break;
         default:
            break;
      }
   }

   /** Check if player's action is valid */
   bool Game::isActionValid(const PlayerAction& action, std::string& reason)
   {
      try
      {
         std::stringstream ss;

         if (getBoardState()->isFinished())
         {
            ss << "Game finished. Please restart another game.";
            throw util::Exception(ss.str());
         }

         if (mCurrentPlayer != action.playerId)
         {
            ss << "Illegal move player " << (int) action.playerId << ": Not your turn!";
            throw util::Exception(ss.str());
         }

         BoardMap map;
         mBoardState->createBoardMap(map, action.playerId);

         if (action.actionType == ActionType::Move)
         {
            Position currentPos = mBoardState->getPlayers(action.playerId).at(action.playerId).position;
            Position p1 = currentPos * 2;
            Position p2 = action.position * 2;
            uint8_t dist = action.position.dist(currentPos);

            if (map(p2) == BoardMap::Invalid)
            {
               ss << "Illegal move player " << (int) action.playerId << ": Cannot move outside board's boundaries!";
               throw util::Exception(ss.str());
            }

            if (p1 == p2)
            {
               ss << "Illegal move player " << (int) action.playerId << ": Same place as before!";
               throw util::Exception(ss.str());
            }

            if (map(p2))
            {
               ss << "Illegal move player " << (int) action.playerId << ": Space occupied!";
               throw util::Exception(ss.str());
            }

            if (dist == 1)
            {
               if (map((p1 + p2) / 2))
               {
                  ss << "Illegal move player " << (int) action.playerId << ": You cannot jump over a wall!";
                  throw util::Exception(ss.str());
               }
            }
            else if (dist == 2)
            {
               // Jump over another pawn
               if (p1.x == p2.x)
               {
                  uint8_t mid = (p1.y + p2.y) / 2;

                  // There's no wall between
                  if (map(p1.x, mid - 1) or map(p1.x, mid + 1))
                  {
                     ss << "Illegal move player " << (int) action.playerId << ": You cannot jump over a wall!";
                     throw util::Exception(ss.str());
                  }

                  if (not map(p1.x, mid))
                  {
                     ss << "Illegal move player " << (int) action.playerId << ": You can move only one space!";
                     throw util::Exception(ss.str());
                  }
               }
               else if (p1.y == p2.y)
               {
                  uint8_t mid = (p1.x + p2.x) / 2;

                  // There's no wall between
                  if (map(mid - 1, p1.y) or map(mid + 1, p1.y))
                  {
                     ss << "Illegal move player " << (int) action.playerId << ": You cannot jump over a wall!";
                     throw util::Exception(ss.str());
                  }

                  if (not map(mid, p1.y))
                  {
                     ss << "Illegal move player " << (int) action.playerId << ": You can move only one space!";
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
                     ss << "Illegal move player " << (int) action.playerId << ": You can move only one space!";
                     throw util::Exception(ss.str());
                  }
               }
            }
            else
            {
               ss << "Illegal move player " << (int) action.playerId << ": You can move only one space!";
               throw util::Exception(ss.str());
            }
         }
         else
         {
            // Check board limits
            if (action.position.x >= BOARD_SIZE or action.position.y >= BOARD_SIZE or
               (action.position.x == 0 and action.position.y == 0) or
               (action.position.x == 0 and action.wallOrientation == Orientation::Horizontal) or
               (action.position.y == 0 and action.wallOrientation == Orientation::Vertical) or
               (action.position.x == BOARD_SIZE - 1 and action.wallOrientation == Orientation::Vertical) or
               (action.position.y == BOARD_SIZE - 1 and action.wallOrientation == Orientation::Horizontal))
            {
               ss << "Illegal move player " << (int) action.playerId << ": Wall outside board's boundaries!";
               throw util::Exception(ss.str());
            }

            // Check if it is not intersecting other wall
            if (action.wallOrientation == Orientation::Vertical)
            {
               Position p = action.position * 2 - 1_y;

               if (map(p) or map(p + 1_x) != BoardMap::MidWall or map(p + 2_x))
               {
                  ss << "Illegal move player " << (int) action.playerId << ": Intersecting another wall!";
                  throw util::Exception(ss.str());
               }

               map(p) = map(p + 1_x) = map(p + 2_x) = BoardMap::VertivalWall;
            }
            else
            {
               Position p = action.position * 2 - 1_x;

               if (map(p) or map(p + 1_y) != BoardMap::MidWall or map(p + 2_y))
               {
                  ss << "Illegal move player " << (int) action.playerId << ": Intersecting another wall!";
                  throw util::Exception(ss.str());
               }

               map(p) = map(p + 1_y) = map(p + 2_y) = BoardMap::HorizontalWall;
            }

            // Check if the wall isn't blocking a pawn's path
            for (PlayerId pId = 0; pId < mNumberOfPlayers; ++pId)
            {
               if (not checkPlayerPath(pId, action))
               {
                  ss << "Illegal move player " << (int) action.playerId << ": Wall blocking player's " << (int) pId << " path!";
                  throw util::Exception(ss.str());
               }
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

   /** Checks if the player's path isn't blocked */
   bool Game::checkPlayerPath(const PlayerId playerId, const PlayerAction& action)
   {
      auto players = mBoardState->getPlayers(playerId);
      auto currentPos = players.at(playerId).position * 2;
      std::list<Position> pos { currentPos };
      BoardMap map;

      getBoardState()->createBoardMap(map, playerId);
      map(currentPos * 2) = BoardMap::Invalid;

      // Place the wall
      auto wall = BoardState::Wall { action.position, action.wallOrientation }.rotate(
         4 - static_cast<int>(players.at(action.playerId).initialState));

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

      // Check the path
      auto checkPos = [&](const Position& p) -> bool
      {
         if (p.x == 0)
         {
            return true;
         }

         if(map(p) < BoardMap::HorizontalWall)
         {
            map(p) = BoardMap::Invalid;
            pos.emplace_back(p);
         }

         return false;
      };

      while (not pos.empty())
      {
         auto p = pos.front();
         pos.pop_front();

         if ((map(p + 1_x) == 0 and checkPos(p + 2_x)) or
            (map(p - 1_x) == 0 and checkPos(p - 2_x)) or
            (map(p + 1_y) == 0 and checkPos(p + 2_y)) or
            (map(p - 1_y) == 0 and checkPos(p - 2_y)))
         {
            return true;
         }
      }

      return false;
   }

} // namespace qcore
