#include "BoardState.h"
#include "QcoreUtil.h"

#include <cstring>

using namespace qcore::literals;

namespace qcore
{
   uint8_t& BoardMap::operator() (uint8_t x, uint8_t y)
   {
      if (x < BOARD_MAP_SIZE and y < BOARD_MAP_SIZE)
      {
         return map[x][y];
      }

      invalidPos = Invalid;
      return invalidPos;
   }

   uint8_t BoardMap::operator() (uint8_t x, uint8_t y) const
   {
      if (x < BOARD_MAP_SIZE and y < BOARD_MAP_SIZE)
      {
         return map[x][y];
      }

      return Invalid;
   }

   bool BoardMap::isPawn(const Position& p)
   {
      uint8_t val = operator ()(p);
      return val >= Pawn3 and val <= Pawn0;
   }

   bool BoardMap::isWall(const Position& p)
   {
      uint8_t val = operator ()(p);
      return val == VertivalWall or val == HorizontalWall;
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   BoardState::Wall BoardState::Wall::rotate(const uint8_t rotations) const
   {
      Wall w;

      if (orientation == Orientation::Vertical)
      {
         switch (rotations & 3)
         {
            case 0:
               w = *this;
               break;
            case 1:
               w.orientation = Orientation::Horizontal;
               w.position = Position(position.y, BOARD_SIZE - position.x - 2);
               break;
            case 2:
               w.orientation = Orientation::Vertical;
               w.position = Position(BOARD_SIZE - position.x - 2, BOARD_SIZE - position.y);
               break;
            case 3:
               w.orientation = Orientation::Horizontal;
               w.position = Position(BOARD_SIZE - position.y, position.x);
               break;
            default:
               break;
         }
      }
      else
      {
         switch (rotations & 3)
         {
            case 0:
               w = *this;
               break;
            case 1:
               w.orientation = Orientation::Vertical;
               w.position = Position(position.y, BOARD_SIZE - position.x);
               break;
            case 2:
               w.orientation = Orientation::Horizontal;
               w.position = Position(BOARD_SIZE - position.x, BOARD_SIZE - position.y - 2);
               break;
            case 3:
               w.orientation = Orientation::Vertical;
               w.position = Position(BOARD_SIZE - position.y - 2, position.x);
               break;
            default:
               break;
         }
      }

      return w;
   }

   /** Rotates all coordinates counterclockwise for a number of steps */
   BoardState::Player BoardState::Player::rotate(const uint8_t rotations) const
   {
      return { qcore::rotate(initialState, rotations), position.rotate(rotations) };
   }

   /** Construction */
   BoardState::BoardState(uint8_t players) :
      mFinished(false),
      mWinner(0xFF)
   {
      mPlayers.resize(players);

      // Set initial player position
      if (players == 2)
      {
         mPlayers[0].position = { BOARD_SIZE - 1, BOARD_SIZE / 2 };
         mPlayers[0].initialState = Direction::Down;

         mPlayers[1].position = { 0, BOARD_SIZE / 2 };
         mPlayers[1].initialState = Direction::Up;
      }
      else if (players == 4)
      {
         mPlayers[0].position = { BOARD_SIZE - 1, BOARD_SIZE / 2 };
         mPlayers[0].initialState = Direction::Down;

         mPlayers[1].position = { BOARD_SIZE / 2, BOARD_SIZE - 1 };
         mPlayers[1].initialState = Direction::Right;

         mPlayers[2].position = { 0, BOARD_SIZE / 2 };
         mPlayers[2].initialState = Direction::Up;

         mPlayers[3].position = { BOARD_SIZE / 2, 0 };
         mPlayers[3].initialState = Direction::Left;
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
   std::list<BoardState::Wall> BoardState::getWalls(const PlayerId id) const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      std::list<BoardState::Wall> walls;
      uint8_t rotations = static_cast<int>(mPlayers.at(id).initialState);

      for ( auto& w : mWalls )
      {
         walls.push_back(w.rotate(rotations));
      }

      return walls;
   }

   /** Get player states from the player's perspective */
   std::vector<BoardState::Player> BoardState::getPlayers(const PlayerId id) const
   {
      std::lock_guard<std::mutex> lock(mMutex);
      std::vector<BoardState::Player> players;
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

   /** Move a player on a different position */
   void BoardState::setPlayerPosition(const PlayerId id, const Position& position)
   {
      {
         std::lock_guard<std::mutex> lock(mMutex);

         // Check winning state
         if (position.x == 0)
         {
            mFinished = true;
            mWinner = id;
         }

         // Update position
         Player &player = mPlayers.at(id);
         player.position = position.rotate(4 - static_cast<int>(player.initialState));
      }

      notifyStateChange();
   }

   /** Put a wall on the board */
   void BoardState::addWall(const PlayerId id, const Position& position, Orientation orientation)
   {
      {
         std::lock_guard<std::mutex> lock(mMutex);
         Player &player = mPlayers.at(id);
         mWalls.push_back(Wall{position, orientation}.rotate(4 - static_cast<int>(player.initialState)));
      }

      notifyStateChange();
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
   void BoardState::notifyStateChange()
   {
      for (auto& cb : mStateChangeCb)
      {
         cb();
      }
   }
} // namespace qcore
