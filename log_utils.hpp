#pragma once
#include "stream_utils.hpp"

namespace app_utils
{
  string parsePrettyFunction(string name);
}

#define FUNCTION_NAME parsePrettyFunction(__PRETTY_FUNCTION__)
#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define LOG_INFO(...) app_utils::stream::StreamWriter(std::cout, true /*newline at end*/).write(__VA_ARGS__)
#define LOG_CALL  LOG_INFO(FUNCTION_NAME)
#define LOG_CALL_INFO(...)  LOG_INFO(FUNCTION_NAME, __VA_ARGS__)