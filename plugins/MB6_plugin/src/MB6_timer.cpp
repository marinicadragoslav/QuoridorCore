#include "MB6_timer.h"

namespace qplugin
{
   void MB6_Timer::Start()
   {
      if (not(mIsRunning) and (mTimerMs > 0))
      {
         mTStart = std::chrono::steady_clock::now();
         mIsRunning = true;
      }
   }

   void MB6_Timer::Reset(const uint32_t timerMs)
   {
      mIsRunning = false;
      mTimerMs = timerMs;
   }

   uint32_t MB6_Timer::GetRemainingMs() const
   {
      uint32_t remainingMs = mTimerMs;

      if (mIsRunning)
      {
         std::chrono::time_point<std::chrono::steady_clock> tNow = std::chrono::steady_clock::now();
         uint32_t elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(tNow - mTStart).count();

         if (elapsedMs < mTimerMs)
         {
            remainingMs = mTimerMs - elapsedMs;
         }
         else
         {
            remainingMs = 0;
         }
      }

      return remainingMs;
   }

   bool MB6_Timer::HasElapsed() const
   {
      return (GetRemainingMs() == 0);
   }

} // end namespace