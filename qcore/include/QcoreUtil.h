#ifndef Header_qcore_Util
#define Header_qcore_Util

#include <exception>
#include <string>
#include <iostream>

namespace qcore
{
   namespace util
   {
      class Exception : public std::exception
      {
      public:
         explicit Exception(const std::string &what) : exception(), cause(what) {}
         virtual ~Exception() = default;

         virtual const char* what() const throw() { return cause.c_str(); }

      private:
         std::string cause;
      };

      class Log
      {
      public:
         enum Level
         {
            Error,
            Warn,
            Info,
            Debug,
            Trace
         };

         ~Log() { std::cout << std::endl; }

         template<typename T> Log& operator <<(const T &p)
         {
            std::cout << p;
            return *this;
         }
      };
   }
}

// TODO: Replace this with an actual log library
#define LOG_LEVEL qcore::util::Log::Info

#define LOG(level) if (level <= LOG_LEVEL) qcore::util::Log()

#define LOG_TRACE(DOM) LOG(qcore::util::Log::Trace) << DOM << " [trace] "
#define LOG_DEBUG(DOM) LOG(qcore::util::Log::Debug) << DOM << " [debug] "
#define LOG_INFO(DOM)  LOG(qcore::util::Log::Info)  << DOM << " [info ] "
#define LOG_WARN(DOM)  LOG(qcore::util::Log::Warn)  << DOM << " [warn ] "
#define LOG_ERROR(DOM) LOG(qcore::util::Log::Error) << DOM << " [error] "


#endif // Header_qcore_Util
