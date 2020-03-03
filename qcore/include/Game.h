#ifndef Header_qcore_Game
#define Header_qcore_Game

#include "PlayerAction.h"
#include "BoardState.h"

#include <mutex>
#include <condition_variable>

namespace qcore
{
   class GameServer;

   class Game
   {
      // Encapsulated data members
   private:

      /** Number of players in the game */
      uint8_t mNumberOfPlayers;

      /** Pointer to the game server */
      std::shared_ptr<GameServer> mGameServer;

   protected:

      /** Keeps the current state of the game */
      std::shared_ptr<BoardState> mBoardState;

      /** The player on move */
      PlayerId mCurrentPlayer;

      /** Protection against concurrent access */
      mutable std::mutex mMutex;
      std::condition_variable mCv;

      // Methods
   public:

      /** Construction */
      Game(uint8_t players);

      virtual ~Game() = default;

      /** Sets the game server */
      void setGameServer(std::shared_ptr<GameServer> gameServer);

      /** Returns the number of players in the game */
      uint8_t getNumberOfPlayers() const { return mNumberOfPlayers; }

      /** Returns the current state of the board */
      BoardStatePtr getBoardState() const { return mBoardState; }

      /** Returns the ID of the player on move */
      PlayerId getCurrentPlayer() const;

      /** Waits until the specified player has picked his move */
      void waitPlayerMove(PlayerId playerId);

      /** Validates and sets the next user action */
      virtual bool processPlayerAction(const PlayerAction& action, std::string& reason);

      /** Checks if player's action is valid */
      bool isActionValid(const PlayerAction& action, std::string& reason) const;

   private:

      /** Checks if the player's path isn't blocked */
      bool checkPlayerPath(const PlayerId playerId, const PlayerAction& action) const;

   };

   typedef std::shared_ptr<Game> GamePtr;
}

#endif // Header_qcore_Game
