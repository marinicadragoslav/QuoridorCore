#ifndef H_PLUGIN_MB6_TIMER
#define H_PLUGIN_MB6_TIMER

#include <chrono>

#define INFINITE_TIME 0xFFFFFFFFU // milliseconds

namespace qplugin
{
   class MB6_Timer
   {
      public:
         MB6_Timer()
         {
            mTimerMs = INFINITE_TIME;
         }
         void Reset(const uint32_t timerMs);
         void Start();
         uint32_t GetRemainingMs() const;
         bool HasElapsed() const;

      private:
         uint32_t mTimerMs;
         bool mIsRunning = false;
         std::chrono::time_point<std::chrono::steady_clock> mTStart;
   };// end class MB6_Timer

} // end namespace

#endif // H_PLUGIN_MB6_TIMER
