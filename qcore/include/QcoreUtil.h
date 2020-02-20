#ifndef Header_qcore_Util
#define Header_qcore_Util

#include <exception>
#include <string>

// TODO: Replace this with an actual log library
#include <iostream>

#define LOG_DEBUG(DOM) std::cout << DOM << " [debug] "
#define LOG_INFO(DOM)  std::cout << DOM << " [info ] "
#define LOG_ERROR(DOM) std::cout << DOM << " [error] "
#define LOG_WARN(DOM)  std::cout << DOM << " [warn ] "

namespace qcore
{
   namespace util
   {
      class Exception : public std::exception
      {
      public:
         explicit Exception(const std::string &what) : exception(), cause(what) {}
         virtual ~Exception() throw () {};

         virtual const char* what() const throw () { return cause.c_str(); }

      private:
         std::string cause;
      };
   }
}

#endif // Header_qcore_Util
