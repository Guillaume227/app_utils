#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <app_utils/cond_check.hpp>

namespace app_utils {

namespace strutils {
constexpr char const* floatRegex = R"([+-]?(:?(:?\d+\.?\d*)|(:?\.\d+)))";

std::string toUpper(std::string in);
std::string toLower(std::string in);

bool startswith(std::string_view src, std::string_view prefix);
bool endswith(std::string_view src, std::string_view suffix);
bool startswith(std::string_view src, char prefix);
bool endswith(std::string_view src, char suffix);
bool contains(std::string_view src, std::string_view substr);
bool contains(std::string_view src, char c);

// Strips any character in 'whitespace' from both ends of 'src' and returns
// a (new) string.
std::string_view strip(std::string_view str, std::string_view whitespace = " \n\t\r");
std::string_view lastLine(std::string_view src);

template <typename T, typename U>
std::string join(T const& separator, std::vector<T> const& v) {
  if (v.empty()) return "";

  std::ostringstream oss;
  oss << v.front();
  for (size_t i = 1; i < v.size(); i++) oss << separator << v[i];

  return oss.str();
}

inline std::ostream& left_justify(std::ostream& os, unsigned width, std::string_view str) {
  return os << std::left << std::setw(width) << str;
}

inline std::ostream& right_justify(std::ostream& os, unsigned width, std::string_view str) {
  return os << std::left << std::setw(width) << str;
}

size_t split(std::string_view str,
             char delim,
             std::string_view& outBuffer,
             size_t maxNumTokens);


std::vector<std::string_view> split(
  char delim, 
  std::string_view str, 
  bool stripWhiteSpace = true,
  int nSplits = -1);

std::vector<std::string_view> splitNoEmpty(char delim, std::string_view str);

// multi-char delimiter
std::vector<std::string_view> splitM(std::string_view delim, std::string_view str);

// Splits the supplied string s by the given regex delim, and the results are
// populated into the return vector.
std::vector<std::string> splitByRegex(std::string const& s, char const* delim);

// Splits respecting balanced brackets:
// i.e. "(1, 2), (3, 4)" will return "(1, 2)", "(3, 4)" and not "(1", "2)", "(3", "4)"
// Recognized brackets include: "()", "{}", "<>", and "[]"
//
// **NOTE**: empty input std::string will result in an empty vector
//

std::vector<std::string_view> splitParse(std::string_view valStr, char delimiter, bool doStrip, int numSplits);

std::vector<std::string_view> splitParse(std::string_view valStr, char delimiter = ',', bool doStrip = true);

bool enclosedInBraces(std::string_view valStr, std::string_view braces = "{}");
std::string_view stripBraces(std::string_view valStr, std::string_view braces = "{}");
std::string_view stripVectorBraces(std::string_view valStr);

std::size_t findMatchingClose(std::string_view valStr, size_t startFrom);
// constexpr std::string length computation
template <size_t N>
constexpr size_t length(char const (&)[N]) {
  return N - 1;
}

char getCloseSymbol(char openSymbol);

inline bool hasOnlyDigits(std::string_view s) { return s.find_first_not_of("0123456789") == std::string::npos; }

void replaceAll(std::string& str, std::string_view from, std::string_view to);

template<typename T>
std::string to_string(T const& val) {
  return std::to_string(val);
}

inline std::string to_string(std::string const& val) {
  return val;
}

inline std::string to_string(std::string_view val) {
  return std::string{val};
}

template <size_t N>
std::string to_string(char const (&val)[N] ) {
  return val;
}

inline std::string to_string(bool b) {
  return b ? "true" : "false";
}

template<typename T>
std::string contiguous_items_to_string(T const* vals, size_t num_items) {
  std::string res = "{";
  for (size_t i = 0; i < num_items; ++i) {
    if (i != 0) {
      res += ", ";
    }
    res += to_string(vals[i]);
  }
  res += '}';
  return res;
}

template <typename T, size_t N>
std::string to_string(std::array<T, N> const& val) {
  return contiguous_items_to_string(val.empty() ? nullptr : &val.front(), val.size());
}

template <typename T>
std::string to_string(std::vector<T> const& val) {
  return contiguous_items_to_string(val.empty() ? nullptr : &val.front(), val.size());
}

inline void from_string(std::string& val, std::string_view val_str) {
  val = std::string{val_str};
}

inline void from_string(std::string_view& val, std::string_view val_str) {
  val = val_str;
}

template<typename T>
requires(std::is_integral_v<T>)
inline void from_string(T& val, std::string_view val_str) {
  val = std::stoi(val_str.data());
}

inline void from_string(float& val, std::string_view val_str) {
  val = std::stof(val_str.data());
}

template <typename T, size_t N>
void from_string(std::array<T, N>& val, std::string_view val_str) {
  std::vector<std::string_view> vals_str = splitParse(val_str, ',');
  checkCond(val.size() == vals_str.size());
  for(size_t i = 0; i < vals_str.size(); i++) {
    from_string(val[i], vals_str[i]);
  }
}

template <typename T>
void from_string( std::vector<T>& val, std::string_view val_str) {
  val.clear();
  std::vector<std::string_view> vals_str = splitParse(val_str, ',');
  val.resize(vals_str.size());
  for(size_t i = 0; i < vals_str.size(); i++) {
    from_string(val[i], vals_str[i]);
  }
}

}  // namespace strutils
}  // namespace app_utils
