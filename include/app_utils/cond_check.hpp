﻿#pragma once

#include "rtti_check.hpp"

#ifdef RTTI_ENABLED

#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>
#include "stream_utils.hpp"

namespace app_utils {

std::vector<std::string> getBackTrace(unsigned int max_stack_depth);
std::string combineTraces(std::string trace1, std::string const& trace2);
std::string_view stripTrace(std::string_view exception_message);

struct Exception : public std::runtime_error {
  static std::string formatStackInfo(char const* file, size_t line, char const* functionName);

  template <typename First, typename... Args,
            typename = std::enable_if_t<not std::is_same<std::decay_t<First>, Exception>::value>>
  Exception(First&& arg, Args&&... args)
      : std::runtime_error(app_utils::make_string(std::forward<First>(arg), std::forward<Args>(args)...)) {}

  friend std::ostream& operator<<(std::ostream& os, Exception const& exc) { return os << exc.what(); }
};

// to use as a terminateHandler
[[noreturn]] void handleUncaughtException() noexcept;
}  // namespace app_utils

#define throwWithTrace(ExceptionT, ...) \
  throw ExceptionT(app_utils::Exception::formatStackInfo(__FILE__, __LINE__, __FUNCTION__) __VA_OPT__(,) __VA_ARGS__)

#define throwExc(...) throwWithTrace(app_utils::Exception, __VA_ARGS__)

#define rethrowExc(app_utilsException, ...)                                                              \
  throw app_utils::Exception(                                                                            \
      app_utils::combineTraces(app_utils::Exception::formatStackInfo(__FILE__, __LINE__, __FUNCTION__) + \
                                   app_utils::stream::StreamWriter::writeStr(__VA_ARGS__),               \
                               app_utilsException.what()))

#define checkCond(condition, ...) \
  do {                            \
    if (not(condition)) {         \
      throwExc(__VA_ARGS__);      \
    }                             \
  } while (false)

#ifdef NDEBUG
#define checkCondDebugOnly(...)
#else
#define checkCondDebugOnly(...) checkCond(__VA_ARGS__)
#endif

// Checks two values compare equal using operator==
#define checkCompareEqual(val1, val2) \
  do {                                \
    if (not(val1 == val2)) {          \
      throwExc(val1, "!=", val2);     \
    }                                 \
  } while (false)

#else
#define checkCond(...)
#define checkCompareEqual(...)
#define throwWithTrace(...)
#define throwExc(...)
#endif
