#ifndef Header_qcore_GameController
#define Header_qcore_GameController

#include "Player.h"
#include "BoardState.h"

#include <string>
#include <vector>
#include <memory>
#include <thread>

namespace qcore
{
   class GameController
   {
      // Encapsulated data members
   private:

      /** Keeps an instance of the game, once initialized */
      std::shared_ptr<Game> mGame;

      /** List of players for the current game */
      std::vector<std::shared_ptr<Player>> mPlayers;

      /** Thread handling player operations */
      std::thread mThread;

      // Methods
   public:

      /** Construction */
      GameController(const std::string& configFile = "quoridor.ini");

      /** Initializes a new local game */
      void initLocalGame(uint8_t numberOfPlayers = 2);

      /** Adds a new player to the game, with the plugin defining his behavior */
      void addPlayer(const std::string& plugin, const std::string& playerName);

      /** Starts the game */
      void start();

      /** Returns the current game state */
      BoardStatePtr getBoardState() const;

      /** Returns the player on move */
      PlayerPtr getCurrentPlayer();

      //
      // Perform actions for the current player. Positions / Directions are relative to the player 0.
      //

      bool moveCurrentPlayer(Direction direction);
      bool moveCurrentPlayer(Position position);
      bool placeWallForCurrentPlayer(Position position, Orientation orientation);

      //
      // TODOs
      //

      void startServer() {}

      void discoverRemoteGames() {}
   };
}

#endif // Header_qcore_GameController
