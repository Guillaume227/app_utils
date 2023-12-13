#pragma once

#include <stdint.h>
#include <chrono>
#include <string>
#include <sstream>
#include "cond_check.hpp"
#include "type_name.hpp"

namespace app_utils
{
  namespace time
  {    
    namespace chrono = std::chrono;

    template <typename T, typename... Others>
    concept one_of = (... or std::same_as<T, Others>);

    template<typename T>
    concept ChronoType = one_of<T,
            chrono::nanoseconds,
            chrono::microseconds,
            chrono::milliseconds,
            chrono::seconds,
            chrono::minutes,
            chrono::hours>;

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
  }  // namespace time  
}  // namespace app_utils

namespace std {

#if defined(__GNUG__) 
// as of GCC 10.2.1 there is still no support for output stream operator on chrono types
// so we provide an implementation
template <typename Duration, typename Ghost = typename app_utils::time::DurationTraits<Duration>::Type>
  inline ostream& operator<<(ostream& os, Duration const& v) {
    return os << v.count() << app_utils::time::DurationTraits<Duration>::units();
  }
#endif

  istream& operator >>(istream& is, chrono::nanoseconds& v);
  istream& operator >>(istream& is, chrono::microseconds& v);
  istream& operator >>(istream& is, chrono::milliseconds& v);
  istream& operator >>(istream& is, chrono::seconds& v);
  istream& operator >>(istream& is, chrono::minutes& v);
  istream& operator >>(istream& is, chrono::hours& v);
}

namespace app_utils::stream {

template<app_utils::time::ChronoType T>
struct StreamPrinter<T> {
  static ostream& toStream(ostream& os, T const& param) {
    return os << app_utils::time::formatDuration(param);
  }
};
}
