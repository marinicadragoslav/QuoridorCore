#include "BoardState.h"
#include "QcoreUtil.h"

#include <cstring>

using namespace qcore::literals;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::BS";

   BoardMap::BoardMap(const BoardMap &from)
   {
      std::memcpy(map, from.map, sizeof(map));
      invalidPos = Invalid;
   }

   uint8_t& BoardMap::operator() (int8_t x, int8_t y)
   {
      if (x >= 0 and y >= 0 and x < BOARD_MAP_SIZE and y < BOARD_MAP_SIZE)
      {
         return map[x][y];
      }

      invalidPos = Invalid;
      return invalidPos;
   }

   uint8_t BoardMap::operator() (int8_t x, int8_t y) const
   {
      if (x >= 0 and y >= 0 and x < BOARD_MAP_SIZE and y < BOARD_MAP_SIZE)
      {
         return map[x][y];
      }

      return Invalid;
   }

   bool BoardMap::isPawn(const Position& p) const
   {
      uint8_t val = operator ()(p);
      return val >= Pawn3 and val <= Pawn0;
   }

   bool BoardMap::isWall(const Position& p) const
   {
      uint8_t val = operator ()(p);
      return val == VertivalWall or val == HorizontalWall;
   }

   bool BoardMap::isPawnSpace(const Position& p) const
   {
       return p.x % 2 == 0 and p.y % 2 == 0;
   }

   /** Construction */
   BoardState::BoardState(uint8_t players, uint8_t walls) :
      mFinished(false),
      mWinner(0xFF)
   {
      mPlayers.resize(players);

      // Set initial player position
      if (players == 2)
      {
         if (walls == 0)
         {
            walls = 10;
         }

         mPlayers[0].position = { BOARD_SIZE - 1, BOARD_SIZE / 2 };
         mPlayers[0].initialState = Direction::Down;
         mPlayers[0].wallsLeft = walls;

         mPlayers[1].position = { 0, BOARD_SIZE / 2 };
         mPlayers[1].initialState = Direction::Up;
         mPlayers[1].wallsLeft = walls;
      }
      else if (players == 4)
      {
         if (walls == 0)
         {
            walls = 5;
         }

         mPlayers[0].position = { BOARD_SIZE - 1, BOARD_SIZE / 2 };
         mPlayers[0].initialState = Direction::Down;
         mPlayers[0].wallsLeft = walls;

         mPlayers[1].position = { BOARD_SIZE / 2, BOARD_SIZE - 1 };
         mPlayers[1].initialState = Direction::Right;
         mPlayers[1].wallsLeft = walls;

         mPlayers[2].position = { 0, BOARD_SIZE / 2 };
         mPlayers[2].initialState = Direction::Up;
         mPlayers[2].wallsLeft = walls;

         mPlayers[3].position = { BOARD_SIZE / 2, 0 };
         mPlayers[3].initialState = Direction::Left;
         mPlayers[3].wallsLeft = walls;
      }
      else
      {
         throw util::Exception( "Invalid number of players" );
      }
   }

   /** Registers callback for state change notification */
   void BoardState::registerStateChange(StateChangeCb cb) const
   {
      mStateChangeCb.push_back(cb);
   }

   /** Get wall states from the player's perspective */
   std::list<WallState> BoardState::getWalls(const PlayerId id) const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      std::list<WallState> walls;
      uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

      for ( auto& w : mWalls )
      {
         walls.push_back(w.rotate(rotations));
      }

      return walls;
   }

   /** Get player states from the player's perspective */
   std::vector<PlayerState> BoardState::getPlayers(const PlayerId id) const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      std::vector<PlayerState> players;
      uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

      for ( auto& p : mPlayers )
      {
         players.push_back(p.rotate(rotations));
      }

      return players;
   }

   /** Check if the specified space is occupied by a pawn */
   bool BoardState::isSpaceEmpty(const Position& position, const PlayerId id) const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

      for (auto &player : mPlayers)
      {
         if (player.rotate(rotations).position == position)
         {
            return false;
         }
      }

      return true;
   }

   /** Flags if the game has finished */
   bool BoardState::isFinished() const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      return mFinished;
   }

   /** Returns the ID of the player who won the game. Valid only when the game has finished. */
   PlayerId BoardState::getWinner() const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      return mWinner;
   }

   /** Returns the last action made */
   PlayerAction BoardState::getLastAction() const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      return mLastAction;
   }

   /** Sets the specified action on the board, after it has been validated */
   void BoardState::applyAction(const PlayerAction& action)
   {
      std::lock_guard<std::mutex> lock(mMutex);
      PlayerState &player = mPlayers.at(action.playerId);
      mLastAction = action.rotate(4 - static_cast<int>(player.initialState));

      switch (action.actionType)
      {
         case ActionType::Move:
         {
            player.position = mLastAction.playerPosition;
            LOG_INFO(DOM) << "Moved player " << (int) action.playerId << " to (" << (int) player.position.x << ", " << (int) player.position.y << ")";

            // Check winning state
            if (action.playerPosition.x == 0)
            {
               mFinished = true;
               mWinner = action.playerId;
               LOG_INFO(DOM) << "Game finished. Player " << (int) action.playerId << " won.";
            }

            break;
         }
         case ActionType::Wall:
         {
            if (player.wallsLeft)
            {
               --player.wallsLeft;
            }

            mWalls.push_back(mLastAction.wallState);

            LOG_INFO(DOM) << "Placed wall by player " << (int) action.playerId << " at ("
               << (int) mLastAction.wallState.position.x << ", " << (int) mLastAction.wallState.position.y << ", "
               << (mLastAction.wallState.orientation == Orientation::Vertical ? "V" : "H") << ")";

            break;
         }
         default:
            break;
      }
   }

   /**
    * Creates a map representing the elements on the board. Between 'pawn' rows / columns are
    * inserted 'wall' rows / columns, therefore the map size will be BOARD_SIZE * 2 - 1.
    */
   void BoardState::createBoardMap(BoardMap& map, const PlayerId id) const
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

   /** Notifies all listeners that the board state has changed */
   void BoardState::notifyStateChange() const
   {
      for (auto& cb : mStateChangeCb)
      {
         cb();
      }
   }
} // namespace qcore
