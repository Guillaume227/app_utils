#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <bitset>
#include <complex>
#include <app_utils/cond_check.hpp>
#include <app_utils/type_name.hpp>

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
std::string_view& strip_in_place(std::string_view& str, std::string_view whitespace = " \n\t\r");
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
  requires requires(T x) { std::to_string(x); }
std::string to_string(T const& val) {
  return std::to_string(val);
}

std::string to_string(double val);
std::string to_string(float val);

template<typename T>
std::string to_string(std::complex<T> const& val) {
  auto& tab = reinterpret_cast<T const(&)[2]>(val);
  return to_string(tab[0]) + "+" + to_string(tab[1]) + "i";
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

template<typename Iterable>
std::string range_to_string(Iterable const& iterable) {
  std::string res = "[";
  auto begin_it = std::begin(iterable);
  for (auto it = begin_it; it != std::end(iterable); it++) {
    if (it != begin_it) {
      res += ", ";
    }
    res += to_string(*it);
  }
  res += ']';
  return res;
}

template <typename T, size_t N>
std::string to_string(std::array<T, N> const& val) {
  return range_to_string(val);
}

template <size_t N>
std::string to_string(std::array<char, N> const& val) {
  auto size = std::min(N, std::strlen(&val.front()));
  return {&val.front(), &val.front() + size};
}

template <typename T>
std::string to_string(std::vector<T> const& val) {
  return range_to_string(val);
}

template <size_t N>
std::string to_string(std::bitset<N> const& val) {
  std::string out(N, '0');
  for (size_t i = 0; i < N; i++) {
    out[i] = val.test(N-1-i) ? '1' : '0'; // note the convention: left to right
  }
  return out;
}

inline std::string to_string(std::vector<char> const& val) {
  return {&val.front(), &val.front() + val.size()};
}

inline bool from_string(std::string& val, std::string_view val_str) {
  val = std::string{val_str};
  return true;
}

inline bool from_string(std::string_view& val, std::string_view val_str) {
  val = val_str;
  return true;
}

inline bool from_string(bool& b, std::string_view val_str) {
  if (val_str == "true") {
    b = true;
  } else if (val_str == "false") {
    b = false;
  } else {
    size_t last_converted_pos = 0;
    int val = std::stoi(val_str.data(), &last_converted_pos);
    checkCond(last_converted_pos == val_str.size(), "failed converting", val_str, "to bool");
    checkCond(val == 1 or val == 0);
    b = val == 1;
  }
  return true;
}

template<typename T>
requires(std::is_integral_v<T>)
inline bool from_string(T& val, std::string_view val_str) {
  size_t last_converted_pos = 0;
  val = static_cast<T>(std::stoi(val_str.data(), &last_converted_pos));
  checkCond(last_converted_pos == val_str.size(), "failed converting", val_str, "to", typeName<T>());
  return true;
}

inline bool from_string(float& val, std::string_view val_str) {
  size_t last_converted_pos = 0;
  val = std::stof(val_str.data(), &last_converted_pos);
  checkCond(last_converted_pos == val_str.size(), "failed converting", val_str, "to float");
  return true;
}

inline bool from_string(double& val, std::string_view val_str) {
  size_t last_converted_pos = 0;
  val = std::stod(val_str.data(), &last_converted_pos);
  checkCond(last_converted_pos == val_str.size(), "failed converting", val_str, "to double");
  return true;
}

inline bool from_string(int& val, std::string_view val_str) {
  size_t last_converted_pos = 0;
  val = std::stoi(val_str.data(), &last_converted_pos);
  checkCond(last_converted_pos == val_str.size(), "failed converting", val_str, "to int");
  return true;
}

template<typename T>
bool from_string(std::complex<T>& val, std::string_view val_str) {
  auto mid_pos = val_str.find('+');
  checkCond(mid_pos != std::string::npos, "bad std::complex format");
  checkCond(val_str.back() == 'i');
  std::string_view real_str = val_str.substr(0, mid_pos);
  auto& tab = reinterpret_cast<T(&)[2]>(val);
  from_string(tab[0], real_str);
  std::string_view imag_str = val_str.substr(mid_pos + 1, val_str.size() - mid_pos - 2);
  from_string(tab[1], imag_str);
  return true;
}

template <typename T, size_t N>
bool from_string(std::array<T, N>& val, std::string_view val_str) {
  val_str = stripBraces(val_str, "[]");
  std::vector<std::string_view> vals_str = splitParse(val_str, ',');
  checkCond(val.size() == vals_str.size());
  bool success = true;
  for(size_t i = 0; i < vals_str.size(); i++) {
    success &= from_string(val[i], vals_str[i]);
  }
  return success;
}

template <typename T>
bool from_string(std::vector<T>& val, std::string_view val_str) {
  val.clear();
  std::vector<std::string_view> vals_str = splitParse(val_str, ',');
  val.resize(vals_str.size());
  bool success = true;
  for(size_t i = 0; i < vals_str.size(); i++) {
    success &= from_string(val[i], vals_str[i]);
  }
  return success;
}

template <size_t N>
bool from_string(std::bitset<N>& val, std::string_view val_str) {
  checkCond(val_str.size() <= N);
  val.reset();
  // allow partial deserialization from the low index bits
  // (right-most bits in string representation).
  size_t str_size = val_str.size();
  for(size_t i = 0; i < str_size; i++) {
    char c = val_str[str_size-1-i];
    if (c == '1') {
      val.set(i);
    } else if (c != '0') {
      return false;
    }
  }
  return true;
}

}  // namespace strutils
}  // namespace app_utils
