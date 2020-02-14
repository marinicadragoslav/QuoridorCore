#ifndef Header_qcore_Player
#define Header_qcore_Player

#include "BoardState.h"
#include "PlayerAction.h"

namespace qcore
{
   // Forward declarations
   class Game;
   typedef std::shared_ptr<Game> GamePtr;

   class Player
   {
      // Encapsulated data members
   private:

      /** Player's ID (from 0 to 3) */
      PlayerId mId;

      /** Player's name (could be the algorithm name). Will be displayed in the UI */
      std::string mName;

      /** Pointer to the 'game board' */
      GamePtr mGame;

      // Methods
   public:

      /** Construction */
      Player(PlayerId id, const std::string& name, GamePtr game);

      /** Destruction */
      virtual ~Player() {};

      //
      // Getters
      //

      /** Returns player's ID (from 0 to 3) */
      PlayerId getId() const { return mId; }

      /** Returns player's name */
      std::string getName() const { return mName; }

      /** Returns the BoardState object */
      BoardStatePtr getBoardState() const;

      /** Returns player's position on the board */
      Position getPosition() const;

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
      bool move(uint8_t x, uint8_t y);
      bool move(const Position& position);

      /**
       * Places a wall at the specified coordinates (starting from the top right corner)
       * @return true if the move is allowed
       */
      bool placeWall(uint8_t x, uint8_t y, Orientation orientation);
      bool placeWall(const BoardState::Wall& wall);

      /**
       * Notifies the player to choose his next action. Must be implemented in derived classes to
       * define player's behavior.
       */
      virtual void doNextMove() = 0;
   };

   typedef std::shared_ptr<Player> PlayerPtr;
}

#endif // Header_qcore_Player
