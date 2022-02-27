#pragma once
#include "stream_utils.hpp"
#include <string>

namespace app_utils
{
  class AutoIndent
  {
    static constexpr char const* indentStr = "  ";
    inline static unsigned m_depth = 0u;
  public:
    AutoIndent() { increment(); }
    ~AutoIndent() { decrement(); }

    static void increment() { ++m_depth; }
    static void decrement() { --m_depth; }

    static unsigned getIndentDepth() { return m_depth; }

    template<typename StreamOut>
    static StreamOut& printIndent(StreamOut& os)
    {
      for (unsigned d = 0; d < m_depth; d++)
        os << indentStr;
      return os;
    }
  };

  struct BlockIndent
  {
    bool m_incremIndent = true;
    explicit BlockIndent(std::string const& message, bool incremIndent = true);
    ~BlockIndent();
  };

  std::string parsePrettyFunction(std::string name);
  }
#ifdef WIN32
#define FUNCTION_NAME app_utils::parsePrettyFunction(__FUNCTION__)
#else
#define FUNCTION_NAME app_utils::parsePrettyFunction(__PRETTY_FUNCTION__)
#endif

#define PPCAT_NX(A, B) A##B
#define PPCAT(A, B) PPCAT_NX(A, B)

#define LOG_LINE(...)   app_utils::BlockIndent PPCAT(blockIndent_, __LINE__) (app_utils::stream::StreamWriter::writeStr(__VA_ARGS__), false)
#define LOG_INDENT(...) app_utils::BlockIndent PPCAT(blockIndent_, __LINE__) (app_utils::stream::StreamWriter::writeStr(__VA_ARGS__), true)
#define LOG_LINE_FUNC(...) LOG_LINE(FUNCTION_NAME, __VA_ARGS__)
#define LOG_INDENT_FUNC(...) LOG_INDENT_FUNC(FUNCTION_NAME, __VA_ARGS__)

#define LOG_CALL            LOG_INDENT(FUNCTION_NAME)
#define LOG_CALL_INFO(...)  LOG_INDENT(FUNCTION_NAME, __VA_ARGS__)

#define LOG_NAME_AND_VALUE(var)   LOG_LINE(#var, ":", var);
