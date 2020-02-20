#ifndef Header_qcore_RemoteGame
#define Header_qcore_RemoteGame

#ifdef BOOST_AVAILABLE

#include "Game.h"

#include <thread>
#include <future>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

namespace qcore
{
   // Forward declaration
   class GameController;

   class RemoteGame : public Game
   {
      // Encapsulated data members
   private:

      GameController& mGameController;

      /** Boost IO Service */
      boost::asio::io_service mIoService;
      boost::asio::io_service::work mIoServiceWork;
      std::thread mIoServiceThread;

      /** TCP client socket */
      boost::asio::ip::tcp::socket mSocket;
      boost::array<char, 1024> mRecvData;
      std::string mRecvBuffer;
      std::promise<std::string> mPromiseResponse;

      // Methods
   public:

      /** Construction */
      RemoteGame(GameController& controller, uint8_t players, const std::string& ip);

      virtual ~RemoteGame() = default;

      /** Adds a player to the remote game server */
      PlayerId addRemotePlayer(const std::string& playerName);

      /** Validates and sets the next user action */
      virtual bool processPlayerAction(const PlayerAction& action, std::string& reason) override;

   private:

      /** Sends a message to the remote game server */
      std::string send(const std::string& message);

      /** Handles socket read */
      void handleRead(const boost::system::error_code& error, size_t bytesTransferred);

      /** Processes the message received from the remote game server */
      void processMessage(const std::string& message);
   };

   typedef std::shared_ptr<RemoteGame> RemoteGamePtr;
}

#endif // BOOST_AVAILABLE
#endif // Header_qcore_RemoteGame
