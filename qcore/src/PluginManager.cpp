#include "PluginManager.h"
#include "QcoreUtil.h"

#include <experimental/filesystem>

#ifdef WIN32
#include <windows.h>
const char * const SHARED_LIBRARY_EXT = ".dll";
#else
#include <dlfcn.h>
const char * const SHARED_LIBRARY_EXT = ".so";
#endif

namespace fs = std::experimental::filesystem;

namespace qcore
{
   /** Log domain */
   const char * const DOM = "qcore::PM";

   // TODO: Move lib path should be in config file
   const char * const SHARED_LIBRARY_PATH = "../lib";

   std::map<std::string, PluginManager::ConstructPlayerFun> PluginManager::RegisteredPlugins;

   /** Returns the list of registered plugin names */
   std::list<std::string> PluginManager::GetPluginList()
   {
      std::list<std::string> plugins;

      for (auto& p : RegisteredPlugins)
      {
         plugins.push_back(p.first);
      }

      return plugins;
   }

   bool PluginManager::PluginAvailable(const std::string& plugin)
   {
      return RegisteredPlugins.find(plugin) != RegisteredPlugins.end();
   }

   /** Player Factory method */
   PlayerPtr PluginManager::CreatePlayer(const std::string& plugin, uint8_t id, const std::string& playerName, GamePtr game)
   {
      auto it = RegisteredPlugins.find(plugin);

      if (it == RegisteredPlugins.end())
      {
         throw util::Exception("Invalid plugin name: " + plugin);
      }

      return it->second(id, playerName, game);
   }

   /** Loads libraries and registers all players found */
   void PluginManager::LoadPlayerLibraries()
   {
      // Check all shared libraries from our path
      if (not fs::exists(SHARED_LIBRARY_PATH) and not fs::is_directory(SHARED_LIBRARY_PATH))
      {
         throw util::Exception("Invalid shared library path");
      }

      for (auto& p: fs::directory_iterator(SHARED_LIBRARY_PATH))
      {
         if (p.path().extension() == SHARED_LIBRARY_EXT)
         {
            void* libHandle = nullptr;

#ifdef WIN32
            // TODO Handle shared libraries in windows
            throw util::Exception("Plugin management not available on windows");
#else
            libHandle = dlopen(p.path().c_str(), RTLD_NOW | RTLD_GLOBAL);

            if (not libHandle)
            {
               throw util::Exception("Failed to load library " + p.path().string());
            }

            LOG_INFO(DOM) << "Loading " << p.path();

            // Look for the player registration function
            RegisterPlayerFun registration = (RegisterPlayerFun) dlsym(libHandle, "RegisterQuoridorPlayer");

            if (registration)
            {
               registration();

               // TODO: Keep all libHandle to be freed later
            }
            else
            {
               dlclose(libHandle);
            }
#endif
         }
      }
   }
} // namespace qcore
