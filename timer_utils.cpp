#include "timer_utils.hpp"
#include "time_utils.hpp"

namespace app_utils
{
  namespace timer{
    using namespace std::chrono_literals;

    class AutoIndent
    {
      static constexpr char const* indentStr = "  ";
      inline static unsigned m_depth = 0u;
    public:
      AutoIndent() { increment(); }
      ~AutoIndent() { decrement(); }

      static void increment() { ++m_depth; }
      static void decrement()
      {
        --m_depth;
      }
      static unsigned getIndentDepth() { return m_depth; }

      template<typename StreamOut>
      static StreamOut& printIndent(StreamOut& os)
      {
        for (unsigned d = 0; d < m_depth; d++)
          os << indentStr;
        return os;
      }
    };
    /*
    BlockIndent::BlockINdent(string const& message, bool incremIndent):
    m_incremIndent(incremIndent)
    {
      if (!message.empty())
        AutoIndent::printIndent(log) << message << endl;
      if (incremIndent_)
        AutoIndent::increment();
      
    }
    */
    const BlockTimer::resolution_t BlockTimer::m_env_threshold = []{
      if (auto varStr = std::getenv(TIMER_THRESHOLD_ENV_VAR))
      {
        return time::durationFromStr<BlockTimer::resolution_t>(varStr);
      } else {
        return BlockTimer::resolution_t{ 0 };
      }      
    }();

    BlockTimer::BlockTimer(string description, bool printStart, resolution_t printThreshold)
      : m_description(std::move(description)),
        m_printThreshold(m_env_threshold != m_env_threshold.zero() ? m_env_threshold : printThreshold)
    {
      if(printStart)
      {
        AutoIndent::printIndent(std::cout) << m_description << " ..." << std::endl;
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
          AutoIndent::printIndent(std::cout) << m_description << " took ";
          if (m_printThreshold < 0us)
            elapsed = chrono::duration_cast<chrono::milliseconds>(elapsed);
          std::cout << time::formatDuration(elapsed, 1) << std::endl;
        }
        m_active = false;       
      }
      return elapsed;
    }
  }
}
