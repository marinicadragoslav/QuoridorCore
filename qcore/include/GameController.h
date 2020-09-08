#ifndef Header_qcore_GameController
#define Header_qcore_GameController

#include "Qcore_API.h"
#include "Player.h"
#include "BoardState.h"

#include <string>
#include <map>
#include <memory>
#include <thread>

namespace qcore
{
   // Forward declaration
   class GameServer;
   class RemoteSession;

   /** Remote location of a game server */
   struct Endpoint
   {
      std::string serverName;
      std::string ip;
   };

   class QCODE_API GameController
   {
      // Encapsulated data members
   private:

      /** Keeps an instance of the game, once initialized */
      std::shared_ptr<Game> mGame;

      /** List of players for the current game */
      std::map<PlayerId, std::shared_ptr<Player>> mPlayers;

      /** Thread handling player operations */
      std::thread mThread;

      /** Handles remote operations */
      std::shared_ptr<GameServer> mGameServer;

      /** Specifies if the game is running on a remote server */
      bool mIsRemoteGame;

      // Methods
   public:

      /** Construction */
      GameController(const std::string& configFile = "quoridor.ini");

      /** Initializes a new remote game */
      void startServer(const std::string& serverName, uint8_t numberOfPlayers = 2);

      /** Starts network discovery and returns the list of IPs where game servers are running */
      std::list<Endpoint> discoverRemoteGames();

      void connectToRemoteGame(const std::string& ip);

      /** Initializes a new local game */
      void initLocalGame(uint8_t numberOfPlayers = 2);

      /** Adds a new player to the game, with the plugin defining his behavior */
      PlayerId addPlayer(const std::string& plugin, const std::string& playerName);

      /** Adds a player running on a remote machine */
      PlayerId addRemotePlayer(std::shared_ptr<RemoteSession> client, const std::string& playerName);

      /** Starts the game */
      void start(bool oneStep = false);

      /** Returns the current game state */
      BoardStatePtr getBoardState() const;

      /** Returns the player on move */
      PlayerPtr getCurrentPlayer();

      /** Returns player by ID */
      PlayerPtr getPlayer(PlayerId playerId);

      /** Returns the game object */
      GamePtr getGame();

      const Game exportGame() const;
      void loadGame(const Game& game);

      //
      // Perform actions for the current player. Positions / Directions are relative to the player 0.
      //

      bool moveCurrentPlayer(Direction direction);
      bool moveCurrentPlayer(Position position);
      bool placeWallForCurrentPlayer(Position position, Orientation orientation);
   };
}

#endif // Header_qcore_GameController
