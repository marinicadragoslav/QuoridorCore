#ifdef BOOST_AVAILABLE

#include "PlayerAction.h"
#include "RemoteGame.h"
#include "GameServer.h"
#include "QcoreUtil.h"

using namespace boost::asio;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::RG";

   /** Construction */
   RemoteGame::RemoteGame(GameController& controller, uint8_t players, const std::string& ip) :
      Game(players),
      mGameController(controller),
      mIoServiceWork(mIoService),
      mIoServiceThread([&]{ mIoService.run(); }),
      mSocket(mIoService, ip::tcp::v4())
   {
      LOG_INFO(DOM) << "Connecting to [" << ip << "] ...";

      try
      {
         mSocket.connect(ip::tcp::endpoint(ip::address::from_string(ip), TCP_GAME_PORT));

         mSocket.async_read_some(
            buffer(mRecvData),
            boost::bind(&RemoteGame::handleRead, this, placeholders::error, placeholders::bytes_transferred));

         std::string response = send(std::string {GameServer::Identification} + CLIENT_ID);

         if (response.at(0) != GameServer::ServerResponse and response.substr(1) != SERVER_ID)
         {
            throw util::Exception("Handshake failed");
         }
      }
      catch (...)
      {
         mIoService.stop();
         mIoServiceThread.join();
         throw;
      }
   }

   /** Adds a player to the remote game server */
   PlayerId RemoteGame::addRemotePlayer(const std::string& playerName)
   {
      std::string request = std::string { GameServer::AddRemotePlayer } + playerName;
      std::string response = send(request);

      if (response.at(0) != GameServer::ServerResponse)
      {
         throw util::Exception("Add Remote Player failed: Invalid response");
      }

      if (response.at(1) != 0)
      {
         throw util::Exception("Add Remote Player failed: " + response.substr(1));
      }

      return response.at(2);
   }

   /** Validates and sets the next user action */
   bool RemoteGame::processPlayerAction(const PlayerAction& action, std::string& reason)
   {
      LOG_DEBUG(DOM) << "Sending player action ...";

      std::string request = std::string { GameServer::PlayerAction } + action.serialize();
      std::string response = send(request);
      bool result = false;

      if (response.at(0) != GameServer::ServerResponse)
      {
         LOG_ERROR(DOM) << "Received invalid response";
      }
      else if (response.at(1) != 0)
      {
         reason = response.substr(2);
         LOG_WARN(DOM) << reason;
      }
      else
      {
         result = true;
      }

      return result;
   }

   /** Sends a message to the remote game server */
   std::string RemoteGame::send(const std::string& message)
   {
      LOG_DEBUG(DOM) << "Sending message type [" << (int) message.at(0) << "] size [" << message.size() << "]";

      {
         // Reset the promise
         std::lock_guard<std::mutex> lock(mMutex);
         mPromiseResponse = std::promise<std::string>();
      }

      boost::asio::write(mSocket, buffer(char(message.size()) + message));
      return mPromiseResponse.get_future().get();
   }

   /** Handles socket read */
   void RemoteGame::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
   {
      if (not error)
      {
         mRecvBuffer += std::string(mRecvData.c_array(), bytesTransferred);

         while ((uint8_t) mRecvBuffer[0] >= mRecvBuffer.size() - 1)
         {
            processMessage(mRecvBuffer.substr(1, mRecvBuffer[0]));
            mRecvBuffer.erase(0, mRecvBuffer[0] + 1);
         }

         // TODO: Detect invalid message and clear buffer

         mSocket.async_read_some(
            buffer(mRecvData),
            boost::bind(&RemoteGame::handleRead, this, placeholders::error, placeholders::bytes_transferred));
      }
      else
      {
         LOG_WARN(DOM) << "Client disconnected: " << error.message();

         // TODO: stop game
      }
   }

   /** Processes the message received from the remote game server */
   void RemoteGame::processMessage(const std::string& message)
   {
      try
      {
         char messageType = message.at(0);
         LOG_DEBUG(DOM) << "Received message type [" << (int) messageType << "] size [" << message.size() << "]";

         switch (messageType)
         {
            case GameServer::ServerResponse:
            {
               std::lock_guard<std::mutex> lock(mMutex);
               mPromiseResponse.set_value(message);
               break;
            }

            case GameServer::RequestPlayerNextMove:
            {
               if (message.size() != 2 )
               {
                  throw util::Exception("Invalid message format");
               }

               PlayerId playerId = message.at(1);
               LOG_DEBUG(DOM) << "Calling next action for player " << (int) playerId << " ...";

               {
                  std::lock_guard<std::mutex> lock(mMutex);
                  mCurrentPlayer = playerId;
               }

               mGameController.getPlayer(playerId)->notifyMove();

               break;
            }

            case GameServer::BoardStateUpdate:
            {
               PlayerAction action;
               action.deserialize(message.substr(1));
               mBoardState->applyAction(action);

               break;
            }

            default:
               LOG_ERROR(DOM) << "Unknown message type";
               break;
         }
      }
      catch (std::exception& e)
      {
         LOG_ERROR(DOM) << "Failed to process message: " << e.what();
      }
   }

} // namespace qcore

#endif // BOOST_AVAILABLE
