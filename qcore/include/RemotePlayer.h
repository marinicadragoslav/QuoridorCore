#ifndef Header_qcore_RemotePlayer
#define Header_qcore_RemotePlayer

#include "Player.h"

namespace qcore
{
   class RemoteSession;
   typedef std::shared_ptr<RemoteSession> RemoteSessionPtr;

   class RemotePlayer : public Player
   {
      // Data members
   private:

      RemoteSessionPtr mRemoteSession;

      // Methods
   public:

      /** Construction */
      RemotePlayer(RemoteSessionPtr remoteSession, PlayerId id, const std::string& name, GamePtr game);

      /** Destruction */
      virtual ~RemotePlayer() = default;

      /** Notifies the RemotePlayer to choose his next action */
      virtual void doNextMove() override;
   };
}

#endif // Header_qcore_RemotePlayer
