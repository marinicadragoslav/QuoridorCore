#include "RemotePlayer.h"
#include "GameServer.h"

namespace qcore
{
   /** Construction */
   RemotePlayer::RemotePlayer(RemoteSessionPtr remoteSession, PlayerId id, const std::string& name, GamePtr game) :
      Player(id, name, game),
      mRemoteSession(remoteSession)
   {
   }

   /** Notifies the RemotePlayer to choose his next action */
   void RemotePlayer::doNextMove()
   {
#if BOOST_AVAILABLE
      mRemoteSession->send(std::string({ GameServer::RequestPlayerNextMove, (char) getId() }));
#endif
   }

} // namespace qcore
