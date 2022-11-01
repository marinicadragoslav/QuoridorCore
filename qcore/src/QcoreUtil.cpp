#include "QcoreUtil.h"
#include <fstream>

namespace qcore
{
   /** Log domain */
   const char *const DOM = "qcore::LOG";

   std::ostream *util::Log::logStream = &std::cout;

   util::Log::~Log()
   {
      ss << std::endl;
      *logStream << ss.str();
      logStream->flush();
   }

   void util::Log::init(const std::string &file)
   {
      LOG_INFO(DOM) << "Redirecting logs to [" << file << "]";
      static std::ofstream ofs;

      // Set exceptions to be thrown on failure
      ofs.exceptions(std::ifstream::failbit | std::ifstream::badbit);
      ofs.open(file);

      if (ofs.is_open())
      {
         logStream = &ofs;
      }
   }
}