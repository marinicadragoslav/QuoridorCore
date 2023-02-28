#include "PluginManager.h"
#include "QcoreUtil.h"
#ifdef WIN32
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
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
#ifdef WIN32
   const char * const SHARED_LIBRARY_PATH = ""; // VS writes the dlls in bin folder
#else
   const char* const SHARED_LIBRARY_PATH = "/../lib";
#endif

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
   PlayerPtr PluginManager::CreatePlayer(const std::string& plugin, PlayerId id, const std::string& playerName, GamePtr game)
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
      auto libPath = fs::current_path().concat(SHARED_LIBRARY_PATH);
      const char *envPath = std::getenv("QUORIDOR_PLUGIN_PATH");

      if (envPath)
      {
         libPath = envPath;
      }

      // Check all shared libraries from our path
      if (not fs::exists(libPath) and not fs::is_directory(libPath))
      {
         throw util::Exception("Invalid shared library path [" + libPath.string() + "]");
      }

      for (auto& p: fs::directory_iterator(libPath))
      {
         if (p.path().extension() == SHARED_LIBRARY_EXT)
         {
            void* libHandle = nullptr;

#ifdef WIN32
            libHandle = LoadLibrary(p.path().string().c_str());

            if (not libHandle)
            {
                throw util::Exception("Failed to load library " + p.path().string() + ": Error code " + std::to_string(GetLastError()));
            }
#else
            libHandle = dlopen(p.path().c_str(), RTLD_NOW | RTLD_GLOBAL);

            if (not libHandle)
            {
                throw util::Exception("Failed to load library " + p.path().string() + ": " + dlerror());
            }
#endif

            LOG_INFO(DOM) << "Loading " << p.path();

            // Look for the player registration function
#ifdef WIN32
            RegisterPlayerFun registration = (RegisterPlayerFun)GetProcAddress(static_cast<HINSTANCE>(libHandle), "RegisterQuoridorPlayer");
#else
            RegisterPlayerFun registration = (RegisterPlayerFun)dlsym(libHandle, "RegisterQuoridorPlayer");
#endif

            if (registration)
            {
                registration();

                // TODO: Keep all libHandle to be freed later
            }
            else
            {
#ifdef WIN32
                FreeLibrary(static_cast<HINSTANCE>(libHandle));
#else 
                dlclose(libHandle);
#endif
            }
         }
      }
   }
} // namespace qcore
