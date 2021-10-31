#pragma once

#include <cstdio>
#include <cstddef>
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <sstream>
#include <type_traits>

namespace app_utils
{  
  std::string parseTypeName(std::string paramName, bool minimal = false);
  template<typename T>
  std::string typeName(bool minimal = false) {
    return parseTypeName(typeid(T).name(), minimal);
  }

  template<typename T>
  std::string typeName(T const& t, bool minimal = false) {
    return parseTypeName(typeid(t).name(), minimal);
  }

  namespace stream {
    using std::ostream;


    namespace details {
      template <class T, class = decltype(std::declval<std::ostream>() << std::declval<T>())>
      std::true_type has_output_stream_operator_test(T const&);
      std::false_type has_output_stream_operator_test(...);
    }  // namespace details

    template <class T>
    using has_output_stream_operator =
        decltype(details::has_output_stream_operator_test(std::declval<T const&>()));


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

    template<>
    struct StreamPrinter<std::byte> {
      static ostream& toStream(ostream& os, std::byte const& param) {
        return os << std::hex << static_cast<uint8_t>(param);
      }
    };

    template <>
    struct StreamPrinter<uint8_t const*> {
      static ostream& toStream(ostream& os, uint8_t const* const& param) { 
        return os << param; }
    };

    template <typename U>
    struct StreamPrinter<std::basic_string_view<uint8_t, U>> {
      static ostream& toStream(ostream& os, std::basic_string_view<uint8_t, U> const& param) {
        for (size_t i = 0; i < param.size(); i++) {
          os << std::hex << *(param.data() + i);
        }
        return os;
      }
    };

    template <typename T>
    struct StreamPrinter<std::span<T>> {
      static ostream& toStream(ostream& os, std::span<T> const& param) {
        for (size_t i = 0; i < param.size(); i++) {
          StreamPrinter<std::decay_t<T>>::toStream(os, *(param.data() + i));
        }
        return os;
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
        case ';':
        case ':':
        case ',':
        case '.':
        case ' ':
        case '\'':
        case '"':
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
        case '\'':
        case ' ':
        case '"':
        case '\n':
        case '\t':
          return false;
        default:
          return true;
        }
      }
    };

    using ios_manipulator = std::ios_base& (std::ios_base&);
    template<>
    struct SeparatorRequirement<ios_manipulator> {
      static constexpr bool needs_before(ios_manipulator) { return true; }
      static constexpr bool needs_after(ios_manipulator) { return false; }
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


    template <>
    struct SeparatorRequirement<std::string_view> {
      static constexpr bool needs_before(std::string_view val) {
        return SeparatorRequirement<char>::needs_before(val.front());
      }
      static constexpr bool needs_after(std::string_view val) {
        return SeparatorRequirement<char>::needs_after(val.back());
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

      template<typename TF, typename TS, typename ...TR>
      ostream& write(TF const& first, TS const& second, TR&&... rest)
      {
        StreamPrinter<TF>::toStream(m_out, first);
        if (m_separator and SeparatorRequirement<TF>::needs_after(first) 
                        and SeparatorRequirement<TS>::needs_before(second)) {
          m_out << m_separator;
        }
        
        return write(second, std::forward<TR>(rest)...);
      }

      template<typename T>
      ostream& write(T const& last) {
        StreamPrinter<T>::toStream(m_out, last);
        if (m_writeLine) {
          m_out << std::endl;
        }
        return m_out;
      }

      template<typename ...Ts>
      static std::string writeStr(Ts&&... args)
      {
        std::ostringstream oss;
        StreamWriter(oss).write(std::forward<Ts>(args)...);
        return oss.str();
      }
    };

    template<>
    inline std::string StreamWriter::writeStr() {
      return "";
    }
  }
}