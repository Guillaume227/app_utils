#pragma once


#include "time_utils.hpp"
#include "stream_utils.hpp"

namespace app_utils::time
{
  template<typename Duration>
  void durationFromString(string const& val, Duration& v)
  {
    string::size_type sz;
    auto const num  =[&]{
      try
      {
        return std::stoi(val, &sz);
      }catch(...)
      {
        condCheck("failed converting", val, "to", app_utils::typeName<Duration>());
      }
    }();

    if(sz >= val.size())
    {
      condCheck("missing duration units for value:", val);
    } else
    {
      auto const nextDigitPos = val.find_first_of("0123456789", sz);
      auto const unitStr = val.substr(sz, nextDigitPos - sz);

      using namespace std::chrono;

      bool const converted =
        durationCastNoLoss<Duration, nanoseconds>(Num, unitStr, v) ||
        durationCastNoLoss<Duration, microseconds>(Num, unitStr, v) ||
        durationCastNoLoss<Duration, milliseconds>(Num, unitStr, v) ||
        durationCastNoLoss<Duration, seconds>(Num, unitStr, v) ||
        durationCastNoLoss<Duration, minutes>(Num, unitStr, v) ||
        durationCastNoLoss<Duration, hours>(Num, unitStr, v);

      condCheck(converted, "unsupported duration units:", unitStr);
      if(nextDigitPos != string::npos)
      {
        Duration vv{};
        durationFromString(val.substr(nextDigitPos), vv, noUnitOk);
        v += vv;
      }
    }
  }

}

#define DURATION_INPUT_OPERATOR(T)\
  std::istream& operator>>(std::istream& is, T&v){\
    string val;\
    is >> val;\
    app_utils::time::durationFromString(val, v);\
    return is;\
  }

namespace std{
  DURATION_INPUT_OPERATOR(chrono::nanoseconds)
  DURATION_INPUT_OPERATOR(chrono::microseconds)
  DURATION_INPUT_OPERATOR(chrono::milliseconds)
  DURATION_INPUT_OPERATOR(chrono::seconds)
  DURATION_INPUT_OPERATOR(chrono::minutes)
  DURATION_INPUT_OPERATOR(chrono::hours)
}
#undef DURATION_INPUT_OPERATOR

template<typename DurationIn, typename FirstDuration, typename...RestDurations>
void formatDurationRecurse(std::ostream&, DurationIn d, int significantLevel, bool foundNonZero) {
  if (foundNonZero && significantlevel > 0)
    significantLevel--;

  auto val = chrono::duration_cast<FirstDuration>(d);
  if(val.count()){
    out << val;
    foundNonZero = true;
  }

  if constexpr(sizeof...(RestDurations) > 0){
    auto rest = d - val;
    if (rest.count() > 0 && significantLevel != 0)
      formatDurationRecurse<DurationIn, RestDurations...>(out, rest, significantLevel, foundNonZero);
  }
}

template<typename Duration>
std::string formatDur(Duration duration, int significantLevel)
{
  std::ostringstream oss;
  if(duration.count() == 0)
  {
    oss << duration;
  }else
  {
    using namespace std::chrono;
    formatDurationRecurse<Duration,
      hours,
      minutes,
      seconds,
      milliseconds,
      microseconds>(oss, duration, significantLevel, /*foundNonZero*/ false);

  }
  return oss.str();
}

std::string formatDuration(std::chrono::microseconds duration, int significantLevel)
{
  return formatDur(duration, significantLevel);
}