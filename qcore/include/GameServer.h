#ifndef Header_qcore_GameServer
#define Header_qcore_GameServer

#ifdef BOOST_AVAILABLE

#include <string>
#include <list>
#include <thread>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/array.hpp>

#include "GameController.h"

namespace qcore
{
   const uint16_t UDP_DISCOVERY_PORT = 12666;
   const uint16_t TCP_GAME_PORT = 12666;

   const char * const CLIENT_ID = "2e49f8b8-f09d-4d3a-b494-35d14768e551";
   const char * const SERVER_ID = "b767eacd-64b6-4bbd-9a38-b4788a94b1f7";

   /** Manages a TCP connection to a client */
   class RemoteSession : public std::enable_shared_from_this<RemoteSession>
   {
      // Encapsulated data members
   private:

      /** Reference to the parent */
      GameServer& mServer;

      /** TCP Socket data */
      boost::asio::ip::tcp::socket mSocket;
      boost::array<char, 1024> mRecvData;
      std::string mRecvBuffer;

      // Methods
   public:

      /** Construction */
      RemoteSession(GameServer& server, boost::asio::io_service& ioService);

      /** Returns a reference to the socket */
      boost::asio::ip::tcp::socket& getSocket();

      /** Starts communication */
      void start();

      /** Sends a message to the client */
      void send(const std::string& message);

   private:

      /** Handler socket read */
      void handleRead(const boost::system::error_code& error, size_t bytesTransferred);

      /** Processes a received message */
      void processMessage(const std::string& message);
   };

   typedef std::shared_ptr<RemoteSession> RemoteSessionPtr;

   class GameServer
   {
      // Type definitions
   public:

      enum MessageType
      {
         Identification = 0,
         ServerResponse,
         AddRemotePlayer,
         RequestPlayerNextMove,
         PlayerAction,
         BoardStateUpdate,
      };

      // Encapsulated data members
   private:

      GameController& mGameController;

      std::string mServerName;
      std::list<Endpoint> mDiscoveredEndpoints;

      boost::asio::io_service mIoService;
      boost::asio::io_service::work mIoServiceWork;
      std::thread mIoServiceThread;

      boost::asio::ip::udp::socket mDiscoverySocket;
      boost::array<char, 1024> mDiscoveryRecvData;
      boost::asio::ip::udp::endpoint mDiscoveryRemoteEndpoint;

      boost::asio::ip::udp::socket mBroadcastSocket;
      boost::array<char, 1024> mBroadcastRecvData;
      boost::asio::ip::udp::endpoint mBroadcastRemoteEndpoint;

      boost::asio::ip::tcp::acceptor mAcceptor;
      std::list<RemoteSessionPtr> mClients;

      // Methods
   public:

      /** Construction */
      GameServer(GameController& controller);

      GameController& getGameController() { return mGameController; }

      void startServer(const std::string& serverName);

      std::list<Endpoint> discoverServers();

      void removeClient(RemoteSessionPtr session);

      void send(const std::string& message);

   private:

      void broadcastDiscoverMessage();

      void startDiscoveryServer();

      void startAccept();

      /** Handle received UDP packet */
      void handleBroadcastMessage(const boost::system::error_code &error, std::size_t bytesReceived);
      void handleDiscoveryMessage(const boost::system::error_code &error, std::size_t bytesReceived);

      /** Handle new connection */
      void handleAccept(RemoteSessionPtr session, const boost::system::error_code& error);
   };
}

#endif // BOOST_AVAILABLE
#endif // Header_qcore_GameServer
