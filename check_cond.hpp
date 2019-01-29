#pragma once
#include "stream_utils.hpp"
#include <stdexcept>

namespace app_utils
{
  struct Exception: public std::runtime_error
  {
    template<typename ...Args>
    Exception(Args&&...args) : std::runtime_error(stream::StreamWriter::writeStr(std::forward<Args>(args)...)){}
  };

}


#define checkCond(condition, ...)\
{\
  if (!(condition))\
  {\
  using namespace std::string_literals;\
    throw app_utils::Exception(__VA_ARGS__, \
      "\n thrown from", __FILE__ + ":"s + std::to_string(__LINE__), __FUNCTION__ + "()"s);\
  }\
}

#define throwExc(...) checkCond(false, __VA_ARGS__)
