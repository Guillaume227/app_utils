#pragma once

#include <app_utils/stream_utils.hpp>

#include <chrono>
#include <string>

namespace app_utils
{
  namespace timer{
    namespace chrono = std::chrono;
    using TimePoint = chrono::time_point<chrono::system_clock>;

    inline TimePoint currentTime(){
      return chrono::system_clock::now();
    }

    class BlockTimer
    {
      bool m_active = true;

    public:
      static constexpr char const* TIMER_THRESHOLD_ENV_VAR = "APP_UTILS_TIMER_THRESHOLD";
      using resolution_t = chrono::microseconds;
      static const resolution_t m_env_threshold;

      BlockTimer(BlockTimer const&) = delete;
      BlockTimer& operator=(BlockTimer const&) = delete;

      std::string const m_description;

      resolution_t const m_printThreshold;
      TimePoint m_start_time;

      explicit BlockTimer(std::string description, bool printStart = false, resolution_t printThreshold = resolution_t(0));

      resolution_t stopTime();
      resolution_t timeSinceStart() const
      {
        return chrono::duration_cast<resolution_t>(currentTime() - m_start_time);
      }
      ~BlockTimer(){
        stopTime();
      }
    };
  }
}

#define TIME_BLOCK(...) \
  app_utils::timer::BlockTimer PPCAT(blockTimer_, __LINE__) (app_utils::stream::StreamWriter::writeStr(__VA_ARGS__))

#define TIME_FUNC\
  TIME_BLOCK(FUNCTION_NAME)

#define TIME_FUNC_ARGS(...)\
  TIME_BLOCK(FUNCTION_NAME, ##__VA_ARGS__)
