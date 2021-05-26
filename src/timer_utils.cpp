#include <app_utils/timer_utils.hpp>
#include <app_utils/time_utils.hpp>
#include <app_utils/log_utils.hpp>

namespace app_utils
{
  namespace timer{
    using namespace std::chrono_literals;

    const BlockTimer::resolution_t BlockTimer::m_env_threshold = []{
      if (auto varStr = std::getenv(TIMER_THRESHOLD_ENV_VAR))
      {
        return time::durationFromStr<BlockTimer::resolution_t>(varStr);
      } else {
        return BlockTimer::resolution_t{ 0 };
      }      
    }();

    BlockTimer::BlockTimer(std::string description, bool printStart, resolution_t printThreshold)
      : m_description(std::move(description)),
        m_printThreshold(m_env_threshold != m_env_threshold.zero() ? m_env_threshold : printThreshold)
    {
      if(printStart){
        LOG_LINE(m_description, "...");
      }
      AutoIndent::increment();
      m_start_time = currentTime();
    }

    BlockTimer::resolution_t BlockTimer::stopTime()
    {
      auto elapsed = timeSinceStart();
      if(m_active)
      {
        AutoIndent::decrement();
        if(elapsed >= chrono::abs(m_printThreshold))
        {
          if (m_printThreshold < 0us)
            elapsed = chrono::duration_cast<chrono::milliseconds>(elapsed);

          LOG_LINE(m_description, "took", time::formatDuration(elapsed, 1));
        }
        m_active = false;       
      }
      return elapsed;
    }
  }
}
