#include <app_utils/time_utils.hpp>
#include <app_utils/stream_utils.hpp>
#include <app_utils/cond_check.hpp>
#include <charconv>

namespace app_utils::time
{
  template<typename D1, typename D2>
  bool durationCastNoLoss(int const num, std::string_view const units, D1& to)
  {
    if (DurationTraits<D2>::units() == units)
    {
      D2 from{num};
      to = chrono::duration_cast<D1>(from);
      checkCond(chrono::duration_cast<D2>(to) == from, 
        "lossy conversion of", from, "to", DurationTraits<D1>::prettyName());
      return true;
    }
    return false;
  }


  template<typename Duration>
  void durationFromString(std::string_view const& val, Duration& v)
  {
    std::string::size_type sz;

    auto const num = [&]{
      int converted_int;
      auto result = std::from_chars(val.data(), val.data() + val.size(), converted_int);
      if (result.ec == std::errc::invalid_argument) {
        throwExc("failed converting", val, "to", app_utils::typeName<Duration>());
      }
      sz = result.ptr - val.data();
      return converted_int;
    }();

    if(sz >= val.size())
    {
      throwExc("missing duration units for value:", val);
    } else {
      auto const nextDigitPos = val.find_first_of("0123456789", sz);
      auto const unitStr = val.substr(sz, nextDigitPos - sz);

      using namespace std::chrono;

      bool const converted =
        durationCastNoLoss<Duration, nanoseconds>(num, unitStr, v) ||
        durationCastNoLoss<Duration, microseconds>(num, unitStr, v) ||
        durationCastNoLoss<Duration, milliseconds>(num, unitStr, v) ||
        durationCastNoLoss<Duration, seconds>(num, unitStr, v) ||
        durationCastNoLoss<Duration, minutes>(num, unitStr, v) ||
        durationCastNoLoss<Duration, hours>(num, unitStr, v);

      checkCond(converted, "unsupported duration units:", unitStr);
      if(nextDigitPos != std::string::npos)
      {
        Duration vv{};
        durationFromString(val.substr(nextDigitPos), vv);
        v += vv;
      }
    }
  }


  template<typename DurationIn, typename FirstDuration, typename...RestDurations>
  void formatDurationRecurse(std::ostream& out, DurationIn d, int significantLevel, bool foundNonZero) {
    if (foundNonZero && significantLevel > 0)
      significantLevel--;

    auto val = chrono::duration_cast<FirstDuration>(d);
    if (val.count()) {
      out << val;
      foundNonZero = true;
    }

    if constexpr(sizeof...(RestDurations) > 0) {
      auto rest = d - val;
      if (rest.count() > 0 && significantLevel != 0)
        formatDurationRecurse<DurationIn, RestDurations...>(out, rest, significantLevel, foundNonZero);
    }
  }

  template<typename Duration>
  std::string formatDur(Duration duration, int significantLevel)
  {
    std::ostringstream oss;
    if (duration.count() == 0)
    {
      oss << duration;
    }
    else
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

  std::string formatDuration(std::chrono::nanoseconds duration, int significantLevel){
    return formatDur(duration, significantLevel);
  }
}

#define DURATION_INPUT_OPERATOR(T)\
  std::istream& operator>>(std::istream& is, T&v){\
    std::string val;\
    is >> val;\
    app_utils::time::durationFromString(val, v);\
    return is;\
  }

namespace std {
  DURATION_INPUT_OPERATOR(chrono::nanoseconds)
  DURATION_INPUT_OPERATOR(chrono::microseconds)
  DURATION_INPUT_OPERATOR(chrono::milliseconds)
  DURATION_INPUT_OPERATOR(chrono::seconds)
  DURATION_INPUT_OPERATOR(chrono::minutes)
  DURATION_INPUT_OPERATOR(chrono::hours)
}
#undef DURATION_INPUT_OPERATOR
