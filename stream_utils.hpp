#pragma once

#include <cstdio>
#include <iostream>
#include <string>
#include <sstream>

namespace app_utils
{
  using std::string;
  
  string parseTypeName(string paramName, bool minimal = false);
  template<typename T>
  string typeName(bool minimal=false){
    return parseTypeName(typeid(T).name(), minimmal);
  }

  template<typename T>
  string typeName(T const& t, bool minimal=false)
  {
    return parseTypeName(typeid(t).name(), minimal);
  }

  namespace stream{
    using std::ostream;

    template<typename T>
    struct StreamPrinter {
      static ostream& toStream(ostream& os, T const& param){
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

    class StreamWriter
    {
      ostream& m_out;
      bool m_writeLine;
      char m_separator;
    public:
      StreamWriter(ostream& os, bool writeLIne=false, char separator = ' ') :
      m_out(os), m_writeLine(writeLIne), m_separator(separator){}

      template<typename TF, typename ...TR>
      ostream& write(TF const& f, TR const&... rest)
      {
        StreamPrinter<TF>::toStream(m_out, f);
        if constexpr(sizeof...(rest) == 0)
        {
          if(m_writeLine)
          {
            m_out << std::endl;
          }
          return m_out;
        } else {
          if (m_separator) {
            m_out << m_separator;
          }
          return write(rest...);
        }
      }

      ostream& write() {
        return m_out;
      }

      template<typename ...Ts>
      static string writeStr(Ts&&... args)
      {
        std::ostringstream oss;
        StreamWriter(oss).write<Ts...>(std::forward(args)...);
        return oss.str();
      }
    };

    template<>
    inline string StreamWriter::writeStr() {
      return "";
    }
  }
}