#ifdef BOOST_AVAILABLE

#include "GameServer.h"
#include "Game.h"
#include "QcoreUtil.h"

// Linux only TODO the same for windows
#include <ifaddrs.h>

#include <functional>

using namespace std::literals::chrono_literals;
using namespace boost::asio;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::GS";

   /** Construction */
   RemoteSession::RemoteSession(GameServer& server, boost::asio::io_service& ioService) :
      mServer(server),
      mSocket(ioService)
   {
   }

   /** Returns a reference to the socket */
   boost::asio::ip::tcp::socket& RemoteSession::getSocket()
   {
      return mSocket;
   }

   /** Starts communication */
   void RemoteSession::start()
   {
      std::string ip = mSocket.remote_endpoint().address().to_string();
      LOG_INFO(DOM) << "Client Session [x] connected from [" << ip << "]\n";

      mSocket.async_read_some(
         buffer(mRecvData),
         boost::bind(&RemoteSession::handleRead, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
   }

   /** Sends a message to the client */
   void RemoteSession::send(const std::string& message)
   {
      LOG_DEBUG(DOM) << "Client Session [x]: Sending message type [" << (int) message.at(0) << "] size [" << message.size() << "]\n";
      boost::asio::write(mSocket, buffer(char(message.size()) + message));
   }

   /** Handler socket read */
   void RemoteSession::handleRead(const boost::system::error_code& error, size_t bytesTransferred)
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
            boost::bind(&RemoteSession::handleRead, shared_from_this(), placeholders::error, placeholders::bytes_transferred));
      }
      else
      {
         LOG_WARN(DOM) << "Client Session [x] disconnected: " << error.message() << "\n";
         mServer.removeClient(shared_from_this());
      }
   }

   /** Processes a received message */
   void RemoteSession::processMessage(const std::string& message)
   {
      try
      {
         char messageType = message.at(0);
         LOG_DEBUG(DOM) << "Client Session [x]: Received message type [" << (int) messageType << "] size [" << message.size() << "]\n";

         switch (messageType)
         {
            case GameServer::Identification:
            {
               if (message.substr(1) != CLIENT_ID)
               {
                  throw util::Exception("Client identification failed");
               }

               send(std::string { GameServer::ServerResponse, 0 } + SERVER_ID);
               break;
            }

            case GameServer::AddRemotePlayer:
            {
               auto playerName = message.substr(1);

               if (playerName.empty())
               {
                  throw util::Exception("Incomplete request");
               }

               PlayerId id = mServer.getGameController().addRemotePlayer(shared_from_this(), playerName);
               send({ GameServer::ServerResponse, 0, (char) id });
               break;
            }

            case GameServer::PlayerAction:
            {
               std::string error;
               PlayerAction action;
               action.deserialize(message.substr(1));

               bool result = mServer.getGameController().getGame()->processPlayerAction(action, error);
               send(std::string { GameServer::ServerResponse, (char) not result } + error);
               break;
            }

            default:
               throw util::Exception("Unknown request");
               break;
         }
      }
      catch (std::exception& e)
      {
         LOG_ERROR(DOM) << "Client Session [x]: Request failed. " << e.what() << "\n";
         send(std::string({ GameServer::ServerResponse, 1}) + e.what());
      }
   }

   /** Construction */
   GameServer::GameServer(GameController& controller) :
      mGameController(controller),
      mIoServiceWork(mIoService),
      mIoServiceThread([&]{ mIoService.run(); }),
      mDiscoverySocket(mIoService),
      mBroadcastSocket(mIoService),
      mAcceptor(mIoService)
   {
      LOG_DEBUG(DOM) << "IO service started\n";
   }

   void GameServer::startServer(const std::string& serverName)
   {
      mServerName = serverName;
      startDiscoveryServer();
      startAccept();
   }

   std::list<Endpoint> GameServer::discoverServers()
   {
      LOG_INFO(DOM) << "Starting server discovery ...\n";

      mDiscoveredEndpoints.clear();
      broadcastDiscoverMessage();
      std::this_thread::sleep_for(3s);
      mBroadcastSocket.close();

      return mDiscoveredEndpoints;
   }

   void GameServer::removeClient(RemoteSessionPtr session)
   {
      mClients.remove(session);
   }

   void GameServer::send(const std::string& message)
   {
      for (auto& c : mClients)
      {
         c->send(message);
      }
   }

   void GameServer::broadcastDiscoverMessage()
   {
      mBroadcastSocket.open(ip::udp::v4());
      mBroadcastSocket.set_option(ip::udp::socket::reuse_address(true));
      mBroadcastSocket.set_option(socket_base::broadcast(true));

      mBroadcastSocket.async_receive_from(
         buffer(mBroadcastRecvData),
         mBroadcastRemoteEndpoint,
         bind(&GameServer::handleDiscoveryMessage, this, placeholders::error, placeholders::bytes_transferred));

      auto ipList = listInterfaces();

      for (auto& ip : ipList)
      {
         LOG_DEBUG(DOM) << "Discover on interface [" << ip << "] ...\n";

         ip::udp::endpoint broadcastEndpoint(ip::address_v4::broadcast(ip, ip::address_v4::netmask(ip)), UDP_DISCOVERY_PORT);

         try
         {
            mBroadcastSocket.send_to(buffer(std::string(CLIENT_ID)), broadcastEndpoint);
         }
         catch (...)
         {
            // ignore
         }
      }
   }

   void GameServer::startDiscoveryServer()
   {
      LOG_INFO(DOM) << "Starting discovery server on port " << UDP_DISCOVERY_PORT << " ...\n";

      ip::udp::endpoint endpoint( ip::udp::v4(), UDP_DISCOVERY_PORT );

      mDiscoverySocket.open(endpoint.protocol());
      mDiscoverySocket.bind(endpoint);
      mDiscoverySocket.set_option(ip::udp::socket::broadcast( true ));

      mDiscoverySocket.async_receive_from(
         buffer(mDiscoveryRecvData),
         mDiscoveryRemoteEndpoint,
         bind(&GameServer::handleBroadcastMessage, this, placeholders::error, placeholders::bytes_transferred));
   }

   void GameServer::startAccept()
   {
      LOG_INFO(DOM) << "Starting game server on port " << TCP_GAME_PORT << " ...\n";

      boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), TCP_GAME_PORT);
      mAcceptor = boost::asio::ip::tcp::acceptor(mIoService, endpoint);
      RemoteSessionPtr session = std::make_shared<RemoteSession>(*this, mIoService);

      mAcceptor.async_accept(
         session->getSocket(),
         boost::bind(&GameServer::handleAccept, this, session, boost::asio::placeholders::error));
   }

   void GameServer::handleBroadcastMessage(const boost::system::error_code &error, std::size_t bytesReceived)
   {
      if (not error)
      {
         std::string address = mDiscoveryRemoteEndpoint.address().to_string();
         std::string data( mDiscoveryRecvData.c_array(), bytesReceived );

         LOG_DEBUG(DOM) << "Received UDP broadcast from [" << address << "] data [" << data << "]\n";

         if (data == CLIENT_ID)
         {
            mDiscoverySocket.send_to(buffer(std::string(SERVER_ID) + mServerName), mDiscoveryRemoteEndpoint);
         }
      }
      else
      {
         LOG_ERROR(DOM) << "Handle broadcast failed: " << error.message() << "\n";
      }

      mDiscoverySocket.async_receive_from(
         buffer(mDiscoveryRecvData),
         mDiscoveryRemoteEndpoint,
         bind(&GameServer::handleBroadcastMessage, this, placeholders::error, placeholders::bytes_transferred));
   }

   void GameServer::handleDiscoveryMessage(const boost::system::error_code &error, std::size_t bytesReceived)
   {
      if (not error)
      {
         std::string address = mBroadcastRemoteEndpoint.address().to_string();
         std::string data(mBroadcastRecvData.c_array(), bytesReceived);
         size_t idLen = strlen(SERVER_ID);

         if (data.substr(0, idLen) == SERVER_ID)
         {
            std::string serverName = data.substr(idLen);

            if (not serverName.empty())
            {
               mDiscoveredEndpoints.push_back({ serverName, address });
               LOG_INFO(DOM) << "Discovered quoridor server [" << serverName << "] on [" << address << "]\n";
            }
         }

         mBroadcastSocket.async_receive_from(
            buffer(mBroadcastRecvData),
            mBroadcastRemoteEndpoint,
            bind(&GameServer::handleDiscoveryMessage, this, placeholders::error, placeholders::bytes_transferred));
      }
      else if (error != boost::system::errc::operation_canceled)
      {
         LOG_ERROR(DOM) << "Handle discovery failed: " << error.message() << "\n";
      }
   }

   void GameServer::handleAccept(RemoteSessionPtr session, const boost::system::error_code& error)
   {
      if (not error)
      {
         mClients.push_back(session);
         session->start();
      }
      else
      {
         LOG_ERROR(DOM) << "Handle accept failed: " << error.message() << "\n";
      }

      RemoteSessionPtr newSession = std::make_shared<RemoteSession>(*this, mIoService);

      mAcceptor.async_accept(
         newSession->getSocket(),
         boost::bind(&GameServer::handleAccept, this, newSession, boost::asio::placeholders::error));
   }

   std::vector<ip::address_v4> GameServer::listInterfaces()
   {
      std::vector<ip::address_v4> ipList;

      ifaddrs *interfaces = NULL;
      int rc = getifaddrs(&interfaces);

      if (rc)
      {
         throw util::Exception("Failed to retrieve network interfaces");
      }

      for (ifaddrs *ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next)
      {
         if (ifa->ifa_addr and ifa->ifa_addr->sa_family == AF_INET)
         {
            char host[NI_MAXHOST];
            rc = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

            if (not rc)
            {
               ipList.push_back(ip::address_v4::from_string(host));
            }
         }
      }

      return ipList;
   }

} // namespace qcore

#endif // BOOST_AVAILABLE
