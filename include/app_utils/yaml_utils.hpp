#pragma once

#include <cstdio>
#include <cstring>
#include <iostream>
#include "string_utils.hpp"

namespace app_utils::yaml {

constexpr char const* indent_str = "  ";
constexpr size_t indent_width = strlen(indent_str);
inline static thread_local int _indent_depth = -1;
inline int get_indent_depth() {
  return _indent_depth;
}

struct indenter_t {

  indenter_t() {
    _indent_depth++;
  }

  ~indenter_t() {
    _indent_depth--;
  }
};

inline std::ostream& print_indent(std::ostream& os) {
  for (int i = 0; i < _indent_depth; i++) {
    os << indent_str;
  }
  return os;
}

template<typename T>
std::ostream& to_yaml(T const& value, std::ostream& os) {
  using namespace app_utils::strutils;
  return os << to_string(value) << '\n';
}

template<typename T>
std::ostream& sequence_to_yaml(T const* vals, size_t num_items, std::ostream& os) {
  os << "[";
  for (size_t i = 0; i < num_items; ++i) {
    if (i != 0) {
      os << ", ";
    }
    to_yaml(vals[i], os);
  }
  os << ']';
  return os;
}

template <typename T, size_t N>
std::ostream& to_yaml(std::array<T, N> const& val, std::ostream& os) {
  return sequence_to_yaml(val.empty() ? nullptr : &val.front(), val.size(), os);
}

template <typename T>
std::ostream& to_yaml(std::vector<T> const& val, std::ostream& os) {
  return sequence_to_yaml(val.empty() ? nullptr : &val.front(), val.size(), os);
}

// this method handles single-line yaml constructs and
// defers to the from_string implementation.
template <typename T>
std::istream& from_yaml(T& val, std::istream& is) {
  std::string line;
  std::getline(is, line);
  using namespace app_utils::strutils;
  std::string_view line_view = line;
  // strip comment from end of line
  line_view = line_view.substr(0, line_view.find_first_of('#'));
  from_string(val, strip(line_view));
  return is;
}

}// namespace app_utils::yaml
