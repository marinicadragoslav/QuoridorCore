#include "GameController.h"
#include "Game.h"
#include "Player.h"
#include "QcoreUtil.h"
#include "PluginManager.h"

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::GC";

   /** Construction */
   GameController::GameController(const std::string&)
   {
      LOG_INFO(DOM) << "Initializing GameController ..." << "\n";

      PluginManager::LoadPlayerLibraries();

      // TODO Parse config params
   }

   /** Initializes a new local game */
   void GameController::initLocalGame(uint8_t numberOfPlayers)
   {
      LOG_INFO(DOM) << "Initializing Local Game with " << (int) numberOfPlayers <<" players ..." << "\n";

      // TODO Stop mThread

      mPlayers.clear();
      mGame = std::make_shared<Game>(numberOfPlayers);
   }

   /** Adds a new player to the game, with the plugin defining his behavior */
   void GameController::addPlayer(const std::string& plugin, const std::string& playerName)
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      if (mPlayers.size() == mGame->getNumberOfPlayers())
      {
         throw util::Exception("Maximum number of players reached");
      }

      mPlayers.push_back(PluginManager::CreatePlayer(plugin, mPlayers.size(), playerName, mGame));
   }

   /** Starts the game */
   void GameController::start()
   {
      if (not mGame)
      {
         throw util::Exception("Game not initialized");
      }

      if (mPlayers.size() != mGame->getNumberOfPlayers())
      {
         throw util::Exception("Not all players joined the game");
      }

      mThread = std::thread([&]()
      {
         while(not getBoardState()->isFinished())
         {
            PlayerId currentPlayer = mGame->getCurrentPlayer();

            // Notify the player to make his next move
            mPlayers[currentPlayer]->doNextMove();

            // Wait for the player to decide
            mGame->waitPlayerMove(currentPlayer);
         }

         LOG_INFO(DOM) << "Game finished. Player " << (int) getBoardState()->getWinner() << " won." << "\n";
      });
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

      PlayerId playerId = mGame->getCurrentPlayer();

      if (playerId >= mPlayers.size())
      {
         throw util::Exception("Player not available");
      }

      return mPlayers.at(playerId);
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

      BoardState::Wall w { position, orientation };
      return player->placeWall(w.rotate(static_cast<int>(initialState)));
   }

} // namespace qcore
