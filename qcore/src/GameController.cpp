#include "GameController.h"
#include "QcoreUtil.h"
#include "PluginManager.h"
#include "Game.h"
#include "Player.h"
#include "GameServer.h"
#include "RemoteGame.h"
#include "RemotePlayer.h"

using namespace std::chrono_literals;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::GC";

   /** Constants */
   const auto PLAYER_MIN_TIME_MS = 1000ms;
   const auto PLAYER_MOVE_TIMEOUT_MS = 5000ms;

   /** Construction */
   GameController::GameController(const std::string&) :
      mMoveInProgress(false),
      mIsRemoteGame(false)
   {
      LOG_INIT("quoridor.log");
      LOG_INFO(DOM) << "Initializing GameController ...";

      PluginManager::LoadPlayerLibraries();

#ifdef BOOST_AVAILABLE
      mGameServer = std::make_shared<GameServer>(*this);
#endif

      // TODO Parse config params
   }

   /** Initializes a new remote game */
   void GameController::startServer(const std::string& serverName, uint8_t numberOfPlayers)
   {
      LOG_INFO(DOM) << "Initializing Remote Game server [" << serverName << "] with " << (int) numberOfPlayers << " players ...";

#ifdef BOOST_AVAILABLE
      // TODO Stop mThread

      mPlayers.clear();
      mGame = std::make_shared<Game>(numberOfPlayers);
      mGame->setGameServer(mGameServer);
      mGameServer->startServer(serverName);
#else
      throw util::Exception("Server implementation not available");
#endif
   }

   /** Starts network discovery and returns the list of IPs where game servers are running */
   std::list<Endpoint> GameController::discoverRemoteGames()
   {
#ifdef BOOST_AVAILABLE
      return mGameServer->discoverServers();
#else
      throw util::Exception("Server implementation not available");
#endif
   }

   void GameController::connectToRemoteGame(const std::string& ip)
   {
#ifdef BOOST_AVAILABLE
      // TODO Stop mThread

      mPlayers.clear();
      mGame = std::make_shared<RemoteGame>(*this, 2, ip);
      mIsRemoteGame = true;
#else
      (void) ip;
      throw util::Exception("Server implementation not available");
#endif
   }

   /** Initializes a new local game */
   void GameController::initLocalGame(uint8_t numberOfPlayers)
   {
      LOG_INFO(DOM) << "Initializing Local Game with " << (int) numberOfPlayers <<" players ...";

      // TODO Stop mThread

      mPlayers.clear();
      mGame = std::make_shared<Game>(numberOfPlayers);
   }

   /** Adds a new player to the game, with the plugin defining his behavior */
   PlayerId GameController::addPlayer(const std::string& plugin, const std::string& playerName)
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      if (not PluginManager::PluginAvailable(plugin))
      {
         throw util::Exception("Plugin not available");
      }

      PlayerId playerId = 0;

      if (not mIsRemoteGame)
      {
         if (mPlayers.size() == mGame->getNumberOfPlayers())
         {
            throw util::Exception("Maximum number of players reached");
         }

         playerId = mPlayers.size();
      }
      else
      {
#ifdef BOOST_AVAILABLE
         auto remoteGame = std::dynamic_pointer_cast<RemoteGame>(mGame);
         playerId = remoteGame->addRemotePlayer(playerName);
#endif
      }

      mPlayers[playerId] = PluginManager::CreatePlayer(plugin, playerId, playerName, mGame);

      // TODO: Check player creation failed and notify remote server

      return playerId;
   }

   PlayerId GameController::addRemotePlayer(std::shared_ptr<RemoteSession> remoteSession, const std::string& playerName)
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      if (mPlayers.size() == mGame->getNumberOfPlayers())
      {
         throw util::Exception("Maximum number of players reached");
      }

      PlayerId playerId = mPlayers.size();
      mPlayers[playerId] = std::make_shared<RemotePlayer>(remoteSession, playerId, playerName, mGame);

      return playerId;
   }

   /** Starts the game */
   void GameController::start(bool oneStep)
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      if (mIsRemoteGame)
      {
         throw util::Exception("Game must be started by the remote server");
      }

      if (mPlayers.size() != mGame->getNumberOfPlayers())
      {
         throw util::Exception("Not all players joined the game");
      }

      if (mPlayerThread.joinable())
      {
         // TODO: Stop thread
         mPlayerThread.join();
      }

      if (mWatchThread.joinable())
      {
         mWatchThread.join();
      }

      mPlayerThread = std::thread([&, oneStep]()
      {
         while(not getBoardState()->isFinished())
         {
            PlayerPtr currentPlayer = getCurrentPlayer();

            {
               // Mark action start
               std::lock_guard<std::mutex> lock(mMutex);
               mActionTs = std::chrono::steady_clock::now();
               mMoveInProgress = true;
            }

            // Notify the player to make his next move
            try
            {
               currentPlayer->notifyMove();
            }
            catch (std::exception& e)
            {
               LOG_ERROR(DOM) << "Exception during player move: " << e.what();
            }

            // Wait for the player to decide
            mGame->waitPlayerMove(currentPlayer->getId());

            {
               // Mark action start
               std::lock_guard<std::mutex> lock(mMutex);
               mMoveInProgress = false;
               auto duration = std::chrono::steady_clock::now() - mActionTs;
               auto moveDutationMs = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

               LOG_INFO(DOM) << "Move duration [" << moveDutationMs / 1000.0 << " sec]";
               currentPlayer->setLastMoveDuration(moveDutationMs);

               // Wait a bit to make the game watchable (during quick moves)
               if (duration < PLAYER_MIN_TIME_MS)
               {
                  std::this_thread::sleep_for(PLAYER_MIN_TIME_MS - duration);
               }
            }

            getBoardState()->notifyStateChange();

            if (oneStep)
               break;
         }
      });

      // Enable watchdog
      const char *wdEnv = std::getenv("QUORIDOR_PLAYER_TIMEOUT_DISABLE");
      bool wdDisabled = wdEnv ? std::stoi(wdEnv) : false;

      if (!oneStep && !wdDisabled)
      {
         mWatchThread = std::thread([&]()
         {
            while(not getBoardState()->isFinished())
            {
               std::unique_lock<std::mutex> lock(mMutex);
               if (mMoveInProgress)
               {
                  auto timelimit = mActionTs + PLAYER_MOVE_TIMEOUT_MS;

                  if (std::chrono::steady_clock::now() > timelimit)
                  {
                     LOG_ERROR(DOM) << "Time limit exceeded by player " << (int) mGame->getCurrentPlayer() << "! Game must end.";
                     mGame->end();
                     break;
                  }
                  else
                  {
                     lock.unlock();
                     mGame->waitPlayerMoveUntil(mGame->getCurrentPlayer(), timelimit);
                  }
               }
               else
               {
                  lock.unlock();
                  std::this_thread::sleep_for(1ms);
               }
            }
         });
      }
   }

   /** Returns the current's game state */
   BoardStatePtr GameController::getBoardState() const
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      return mGame->getBoardState();
   }

   /** Returns the player on move */
   PlayerPtr GameController::getCurrentPlayer()
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      return getPlayer(mGame->getCurrentPlayer());
   }

   PlayerPtr GameController::getPlayer(PlayerId playerId)
   {
      auto it = mPlayers.find(playerId);
      if (it == mPlayers.end())
      {
         throw util::Exception("Player " + std::to_string((int) playerId) + " not available");
      }

      return it->second;
   }

   GamePtr GameController::getGame()
   {
      return mGame;
   }

   const Game GameController::exportGame() const
   {
       return *mGame;
   }

   void GameController::loadGame(const Game& game) 
   {
       *mGame = game;

       mGame->restore();
   }

   bool GameController::moveCurrentPlayer(Direction direction)
   {
      auto player = getCurrentPlayer();
      auto initialState = getBoardState()->getPlayers(0).at(player->getId()).initialState;

      return player->move(rotate(direction, static_cast<int>(initialState)));
   }

   bool GameController::moveCurrentPlayer(Position position)
   {
      auto player = getCurrentPlayer();
      auto initialState = getBoardState()->getPlayers(0).at(player->getId()).initialState;

      return player->move(position.rotate(static_cast<int>(initialState)));
   }

   bool GameController::placeWallForCurrentPlayer(Position position, Orientation orientation)
   {
      auto player = getCurrentPlayer();
      auto initialState = getBoardState()->getPlayers(0).at(player->getId()).initialState;

      WallState wall { position, orientation };
      return player->placeWall(wall.rotate(static_cast<int>(initialState)));
   }

} // namespace qcore
