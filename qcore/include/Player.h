#ifndef Header_qcore_Player
#define Header_qcore_Player

#include "Qcore_API.h"
#include "BoardState.h"
#include "PlayerAction.h"
#include <atomic>

namespace qcore
{
   // Forward declarations
   class Game;
   typedef std::shared_ptr<Game> GamePtr;

   class QCODE_API Player
   {
      // Encapsulated data members
   private:

      /** Player's ID (from 0 to 3) */
      PlayerId mId;

      /** Player's name (could be the algorithm name). Will be displayed in the UI */
      std::string mName;

      /** Pointer to the 'game board' */
      GamePtr mGame;

      /** Last move duration, in milliseconds */
      std::atomic_int mLastMoveDurationMs;

      /** Number of illegal moves attempted by the player */
      std::atomic_int mIllegalMoves;

      // Methods
   public:

      /** Construction */
      Player(PlayerId id, const std::string& name, GamePtr game);

      /** Destruction */
      virtual ~Player() = default;

      /** Called by game controller to notify player's next move */
      void notifyMove();

      //
      // Getters
      //

      /** Returns player's ID (from 0 to 3) */
      PlayerId getId() const { return mId; }

      /** Returns player's name */
      std::string getName() const { return mName; }

      /** Returns the duration of the last move */
      uint32_t getLastMoveDuration() const { return mLastMoveDurationMs; }

       /** Sets last move duration - stats purposes only */
      void setLastMoveDuration(int ms) { mLastMoveDurationMs = ms; }

      uint32_t getIllegalMoves() const { return mIllegalMoves; }

      /** Returns the BoardState object */
      BoardStatePtr getBoardState() const;

      /** Returns player's position on the board */
      Position getPosition() const;

      /** Returns number of walls left for the current player */
      uint8_t getWallsLeft() const;

      //
      // Player Actions
      // All coordinates must be set from the player's perspective (relative to the player's start).
      // A player will always start at the bottom and must reach first line of the board.
      //

      /**
       * Performs a 'move' action
       * @return true if the move is allowed
       */
      bool move(Direction direction);
      bool move(int8_t x, int8_t y);
      bool move(const Position& position);

      /**
       * Places a wall at the specified coordinates (starting from the top right corner)
       * @return true if the move is allowed
       */
      bool placeWall(int8_t x, int8_t y, Orientation orientation);
      bool placeWall(const WallState& wall);

      /**
       * Checks if placing the specified wall is allowed
       */
      bool isValid(const WallState& wall) const;

      /**
       * Checks if moving the specified position is allowed
       */
      bool isValid(const Position& position) const;

   private:

      /**
       * Notifies the player to choose his next action. Must be implemented in derived classes to
       * define player's behavior.
       */
      virtual void doNextMove() = 0;
   };

   typedef std::shared_ptr<Player> PlayerPtr;
}

#endif // Header_qcore_Player
