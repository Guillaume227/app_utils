#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>

namespace app_utils
{
  using std::string;
  
  string parseTypeName(string paramName, bool minimal = false);
  template<typename T>
  string typeName(bool minimal=false){
    return parseTypeName(typeid(T).name(), minimal);
  }

  template<typename T>
  string typeName(T const& t, bool minimal=false){
    return parseTypeName(typeid(t).name(), minimal);
  }

  namespace stream {
    using std::ostream;

    template<typename T>
    struct StreamPrinter {
      static ostream& toStream(ostream& os, T const& param) {
        return os << param;
      }
    };

    template<>
    struct StreamPrinter<char const*> {
      static ostream& toStream(ostream& os, char const* const& param) {
        return os << param;
      }
    };

    template<typename T>
    struct StreamPrinter<T*> {
      static ostream& toStream(ostream& os, T const* param) {
        auto paramName = param ? typeid(*param).name() : typeid(param).name();
        return os << parseTypeName(paramName);
      }
    };

    template<>
    struct StreamPrinter<bool> {
      static ostream& toStream(ostream& os, bool param) {
        return os << (param ? "true" : "false");
      }
    };

    template<typename T>
    struct SeparatorRequirement {
      static constexpr bool needs_before(T const&) { return true; }
      static constexpr bool needs_after(T const&) { return true; }
    };

    template<>
    struct SeparatorRequirement<char> {
      static constexpr bool needs_before(char const& val) {
        switch (val) {
        case ')':
        case ']':
        case '}':
        case ' ':
        case ';':
        case ':':
        case ',':
        case '.':
        case '\n':
        case '\t':
          return false;
        default:
          return true;
        }
      }
      static constexpr bool needs_after(char const& val) {
        switch (val) {
        case '(':
        case '[':
        case '{':
        case '\n':
        case '\t':
          return false;
        default:
          return true;
        }
      }
    };

    using io_manipulator = std::ios_base& (std::ios_base& str);
    template<>
    struct SeparatorRequirement<io_manipulator> {
      static constexpr bool needs_before(io_manipulator const&) {
        return true;
      }
      static constexpr bool needs_after(io_manipulator const&) {
        return false;
      }
    };

    template<>
    struct SeparatorRequirement<char const*> {
      static constexpr bool needs_before(char const* const& val) {
        return val && std::strlen(val) > 0 && SeparatorRequirement<char>::needs_before(val[0]);
      }
      static constexpr bool needs_after(char const* const& val) {
        return val && std::strlen(val) > 0 && SeparatorRequirement<char>::needs_after(val[std::strlen(val)-1]);
      }
    };

    template<>
    struct SeparatorRequirement<char[1]> {
      static constexpr bool needs_before(char const (&) [1]) { return false; }
      static constexpr bool needs_after(char const (&) [1]) { return false; }
    };

    template<size_t N>
    struct SeparatorRequirement<char[N]> {
      static constexpr bool needs_before(char const (& val) [N]) {
        return SeparatorRequirement<char>::needs_before(val[0]);
      }
      static constexpr bool needs_after(char const (& val) [N]) {
        return SeparatorRequirement<char>::needs_after(val[N - 2]);
      }
    };


    class StreamWriter
    {
      ostream& m_out;
      bool m_writeLine;
      char m_separator;

    public:
      StreamWriter(ostream& os, bool writeLine=false, char separator = ' ') :
      m_out(os), m_writeLine(writeLine), m_separator(separator){}

      template<typename TF, typename TF2, typename ...TR>
      ostream& write(TF const& f, TF2 const& f2, TR const&... rest)
      {
        StreamPrinter<TF>::toStream(m_out, f);
        if (m_separator && SeparatorRequirement<TF>::needs_after(f) && SeparatorRequirement<TF2>::needs_before(f2)) {
          m_out << m_separator;
        }
        if constexpr(sizeof...(rest) == 0) {
          return write(f2);
        } else {
          return write(f2, rest...);
        }
      }

      template<typename TF>
      ostream& write(TF const& f) {
        StreamPrinter<TF>::toStream(m_out, f);
        if (m_writeLine) {
          m_out << std::endl;
        }
        return m_out;
      }

      template<typename ...Ts>
      static string writeStr(Ts&&... args)
      {
        std::ostringstream oss;
        StreamWriter(oss).write<Ts...>(std::forward<Ts>(args)...);
        return oss.str();
      }
    };

    template<>
    inline string StreamWriter::writeStr() {
      return "";
    }
  }
}