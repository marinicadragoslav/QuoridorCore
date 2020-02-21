#ifndef Header_qcore_GameState
#define Header_qcore_GameState

#include "PlayerAction.h"

#include <list>
#include <vector>
#include <memory>
#include <mutex>
#include <functional>

namespace qcore
{
   const uint8_t BOARD_MAP_SIZE = BOARD_SIZE * 2 - 1;

   class BoardMap
   {
      // Type definitions
   public:
      enum ItemType : uint8_t
      {
         Invalid = 0xFF,
         VertivalWall = 0xFE,
         HorizontalWall = 0xFD,
         Pawn0 = 0xFC,
         Pawn1 = 0xFB,
         Pawn2 = 0xFA,
         Pawn3 = 0xF9,
         MidWall = 0x2D // "-" ASCII
      };

      // Encapsulated data members
   private:

      uint8_t map[BOARD_MAP_SIZE][BOARD_MAP_SIZE] = {};
      uint8_t invalidPos = 0;

      // Methods
   public:

      uint8_t& operator() (uint8_t x, uint8_t y);
      uint8_t operator() (uint8_t x, uint8_t y) const;

      uint8_t& operator() (const Position& p) { return operator()(p.x, p.y); }
      uint8_t operator() (const Position& p) const { return operator()(p.x, p.y); }

      bool isPawn(const Position& p);
      bool isWall(const Position& p);
   };

   class BoardState
   {
      // Type definitions
   public:

      /** Description of a wall on the board */
      struct Wall
      {
         Position position;
         Orientation orientation;

         /** Rotates all coordinates counterclockwise for a number of steps */
         Wall rotate(const uint8_t rotations) const;
      };

      /** Description of a player on the board */
      struct Player
      {
         Direction initialState;
         Position position;

         /** Rotates all coordinates counterclockwise for a number of steps */
         Player rotate(const uint8_t rotations) const;
      };

      typedef std::function<void()> StateChangeCb;

      // Encapsulated data members
   private:

      /** List of walls placed on the board */
      std::list<Wall> mWalls;

      /** List of players and their position */
      std::vector<Player> mPlayers;

      /** Flags if the game has finished */
      bool mFinished;

      /** The player who won */
      PlayerId mWinner;

      /** List of state change callbacks */
      mutable std::list<StateChangeCb> mStateChangeCb;

      /** Protection against concurrent access */
      mutable std::mutex mMutex;

      // Methods
   public:

      /** Construction */
      BoardState(uint8_t players);

      /** Registers callback for state change notification */
      void registerStateChange(StateChangeCb cb) const;

      //
      // Getters over different board information
      // All information are from the perspective of the player set as parameter.
      //

      /** Get wall states */
      std::list<Wall> getWalls(const PlayerId id) const;

      /** Get player states */
      std::vector<Player> getPlayers(const PlayerId id) const;

      /** Check if the specified space is occupied by a pawn */
      bool isSpaceEmpty(const Position& position, const PlayerId id) const;

      /** Flags if the game has finished */
      bool isFinished() const { return mFinished; }

      /** Returns the ID of the player who won the game. Valid only when the game has finished. */
      PlayerId getWinner() const { return mWinner; }

      /**
       * Creates a matrix representing the elements on the board. Between 'pawn' rows / columns are
       * inserted 'wall' rows / columns, therefore the map size will be BOARD_SIZE * 2 - 1.
       */
      void createBoardMap(BoardMap& map, const PlayerId id) const;

      //
      // Board updates (called only from inside the Game).
      // Players will not be able to alter the board state directly.
      //

      /** Move a player on a different position */
      void setPlayerPosition(const PlayerId id, const Position& pos);

      /** Put a wall on the board */
      void addWall(const PlayerId id, const Position& position, Orientation orientation);

   private:

      /** Notifies all listeners that the board state has changed */
      void notifyStateChange();
   };

   typedef std::shared_ptr<const BoardState> BoardStatePtr;
}

#endif // Header_qcore_GameState
