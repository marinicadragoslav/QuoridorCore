#ifndef Header_qcore_Game
#define Header_qcore_Game

#include "PlayerAction.h"
#include "BoardState.h"

#include <mutex>
#include <condition_variable>

namespace qcore
{
   class Game
   {
      // Encapsulated data members
   private:

      /** Keeps the current state of the game */
      std::shared_ptr<BoardState> mBoardState;

      /** Number of players in the game */
      uint8_t mNumberOfPlayers;

      /** The player on move */
      PlayerId mCurrentPlayer;

      /** Protection against concurrent access */
      mutable std::mutex mMutex;
      std::condition_variable mCv;

      // Methods
   public:

      /** Construction */
      Game(uint8_t players);

      /** Returns the number of players in the game */
      uint8_t getNumberOfPlayers() const { return mNumberOfPlayers; }

      /** Returns the current state of the board */
      BoardStatePtr getBoardState() const { return mBoardState; }

      /** Returns the ID of the player on move */
      PlayerId getCurrentPlayer() const;

      /** Waits until the specified player has picked his move */
      void waitPlayerMove(PlayerId playerId);

      /** Validates and sets the next user action */
      bool processPlayerAction(const PlayerAction& action);

   private:

      /** Checks if player's action is valid */
      bool isActionValid(const PlayerAction& action);

      /** Checks if the player's path isn't blocked */
      bool checkPlayerPath(const PlayerId playerId, const PlayerAction& action);
   };

   typedef std::shared_ptr<Game> GamePtr;
}

#endif // Header_qcore_Game
