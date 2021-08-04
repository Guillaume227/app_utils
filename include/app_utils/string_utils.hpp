#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

namespace app_utils {

namespace strutils {
constexpr char const* floatRegex = R"([+-]?(:?(:?\d+\.?\d*)|(:?\.\d+)))";

std::string toUpper(std::string in);
std::string toLower(std::string in);

bool startswith(std::string const& src, std::string const& prefix);
bool endswith(std::string const& src, std::string const& suffix);
bool startswith(std::string const& src, char prefix);
bool endswith(std::string const& src, char suffix);
bool contains(std::string const& src, std::string const& substr);
bool contains(std::string const& src, char c);

// Strips any character in 'whitespace' from both ends of 'src' and returns
// a (new) string.
std::string strip(std::string const& str, std::string const& whitespace = " \n\t\r");
std::string lastLine(std::string const& src);

template <typename T, typename U>
std::string join(T const& separator, std::vector<T> const& v) {
  if (v.empty()) return "";

  std::ostringstream oss;
  oss << v.front();
  for (size_t i = 1; i < v.size(); i++) oss << separator << v[i];

  return oss.str();
}

inline std::ostream& left_justify(std::ostream& os, unsigned width, std::string const& str) {
  return os << std::left << std::setw(width) << str;
}

inline std::ostream& right_justify(std::ostream& os, unsigned width, std::string const& str) {
  return os << std::left << std::setw(width) << str;
}

std::vector<std::string> split(char delim, std::string const& str, bool doStrip = true, bool discardEmpty = true,
                               int nSplits = -1);
std::vector<std::string> splitNoEmpty(char delim, std::string const& str);

// multi-char delimiter
std::vector<std::string> splitM(char const* delim, std::string const& str);

// Splits the supplied string s by the given regex delim, and the results are
// populated into the return vector.
std::vector<std::string> splitByRegex(std::string const& s, char const* delim);

// Splits respecting balanced brackets:
// i.e. "(1, 2), (3, 4)" will return "(1, 2)", "(3, 4)" and not "(1", "2)", "(3", "4)"
// Recognized brackets include: "()", "{}", "<>", and "[]"
//
// **NOTE**: empty input std::string will results in an empty vector
//

std::vector<std::string> splitParse(std::string const& valStr, char delimiter, bool doStrip, int numSplits);

std::vector<std::string> splitParse(std::string const& valStr, char delimiter = ',', bool doStrip = true);

bool enclosedInBraces(std::string const& valStr, std::string const& braces = "{}");
std::string stripBraces(std::string const& valStr, std::string const& braces = "{}");
std::string stripVectorBraces(std::string const& valStr);

std::size_t findMatchingClose(std::string const& valStr, size_t startFrom);
// constexpr std::string length computation
template <size_t N>
constexpr size_t length(char const (&)[N]) {
  return N - 1;
}

char getCloseSymbol(char openSymbol);

inline bool hasOnlyDigits(std::string const& s) { return s.find_first_not_of("0123456789") == std::string::npos; }

void replaceAll(std::string& str, std::string const& from, std::string const& to);


template<typename T>
std::string to_string(T val) {
  return std::to_string(val);
}

template <>
inline std::string to_string<std::string>(std::string val) {
  return val;
 }

template <size_t N>
std::string to_string(char const (&val)[N] ) {
  return val;
}
}  // namespace strutils
}  // namespace app_utils
