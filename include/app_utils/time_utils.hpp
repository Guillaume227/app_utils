#pragma once

#include <stdint.h>
#include <chrono>
#include <string>
#include <sstream>
#include <app_utils/cond_check.hpp>

namespace app_utils
{
  namespace time
  {    
    namespace chrono = std::chrono;

    template<typename Duration>
    struct DurationTraits{};

#define DEFINE_DURATION_TRAITS(Duration, unitsStr)\
    template<>\
    struct DurationTraits<chrono:: Duration>{\
      using Type = chrono:: Duration;\
      static constexpr char const* units() { return #unitsStr; }\
      static constexpr char const* prettyName() { return #Duration;  }\
    }

    DEFINE_DURATION_TRAITS(hours, h);
    DEFINE_DURATION_TRAITS(minutes, min);
    DEFINE_DURATION_TRAITS(seconds, s);
    DEFINE_DURATION_TRAITS(milliseconds, ms);
    DEFINE_DURATION_TRAITS(microseconds, us);
    DEFINE_DURATION_TRAITS(nanoseconds, ns);

#undef DEFINE_DURATION_TRAITS

    std::string formatDuration(chrono::nanoseconds duration, int significantLevel = -1);

    template<typename D=chrono::seconds>
    D durationFromStr(std::string inputStr)
    {
      D d;
      std::istringstream iss(inputStr);
      iss >> d;
      checkCond(iss.eof(), "failed parsing duration", typeName<D>(), "from string '", inputStr, "'");
      return d;
    }
  }
}

namespace std
{

  istream& operator >>(istream& is, chrono::nanoseconds& v);
  istream& operator >>(istream& is, chrono::microseconds& v);
  istream& operator >>(istream& is, chrono::milliseconds& v);
  istream& operator >>(istream& is, chrono::seconds& v);
  istream& operator >>(istream& is, chrono::minutes& v);
  istream& operator >>(istream& is, chrono::hours& v);
}