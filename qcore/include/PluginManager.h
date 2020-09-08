#ifndef Header_qcore_PluginManager
#define Header_qcore_PluginManager

#include "Qcore_API.h"
#include "QcoreUtil.h"
#include "PlayerAction.h"

#include <functional>
#include <memory>
#include <string>
#include <map>
#include <list>

#define REGISTER_QUORIDOR_PLAYER(plugin) extern "C" void RegisterQuoridorPlayer() \
   { qcore::PluginManager::RegisterPlugin<plugin>(#plugin); }

namespace qcore
{
   // Forward declarations
   class Player;
   class Game;
   typedef std::shared_ptr<Player> PlayerPtr;
   typedef std::shared_ptr<Game> GamePtr;

   class QCODE_API PluginManager
   {
      // Type definition
   public:

      typedef std::function<PlayerPtr(PlayerId, const std::string&, GamePtr)> ConstructPlayerFun;
      typedef void (*RegisterPlayerFun)();

      // Encapsulated data members
   private:

      /** List of registered plugins and custom Players constructors */
      static std::map<std::string, ConstructPlayerFun> RegisteredPlugins;

      // Methods
   public:

      template<typename T>
      static void RegisterPlugin(const std::string& name)
      {
         LOG_INFO("qcore::PM") << "Registering Plugin " << name;

         RegisteredPlugins[name] = [](PlayerId id, const std::string& name, GamePtr game)
            { return std::make_shared<T>(id, name, game); };
      }

      /** Returns the list of registered plugin names */
      static std::list<std::string> GetPluginList();

      /** Check if the requested plugin exists */
      static bool PluginAvailable(const std::string& plugin);

      /** Player Factory method */
      static PlayerPtr CreatePlayer(const std::string& plugin, PlayerId id, const std::string& playerName, GamePtr game);

      /** Loads libraries and registers all players found */
      static void LoadPlayerLibraries();
   };
}

#endif // Header_qcore_PluginManager
