#pragma once

#include <cstring>
#include <regex>
#include <sstream>
#include <vector>

#include <app_utils/string_utils.hpp>
#include <app_utils/cond_check.hpp>
#include <app_utils/container_utils.hpp>

namespace app_utils {

namespace strutils {

namespace {
std::string const openSymbols = "<({[";
std::string const closeSymbols = ">)}]";
}  // namespace

bool isCloseSymbol(char c) { return closeSymbols.find(c) != std::string::npos; }
bool isOpenSymbol(char c) { return openSymbols.find(c) != std::string::npos; }

char getCloseSymbol(char openSymbol) {
  auto pos = openSymbols.find(openSymbol);
  if (pos == std::string::npos) {
    throwExc("no close symbol found for", openSymbol);
  }
  return closeSymbols[pos];
}

std::string toUpper(std::string str) {
  for (auto& c : str) c = static_cast<char>(::toupper(c));

  return str;
}

std::string toLower(std::string str) {
  for (auto& c : str) c = static_cast<char>(::tolower(c));

  return str;
}

bool contains(std::string const& src, std::string const& substr) { return src.find(src) != std::string::npos; }

bool contains(std::string const& src, char const c) { return src.find(c) != std::string::npos; }

bool startswith(std::string const& src, std::string const& suffix) {
  auto pos = src.find(suffix);
  return pos == 0;
}

bool startswith(std::string const& src, char const prefix) { return not src.empty() and src.front() == prefix; }

bool endswith(std::string const& src, char const suffix) { return not src.empty() and src.back() == suffix; }

bool endswith(std::string const& src, std::string const& suffix) {
  auto pos = src.rfind(suffix);
  if (pos != std::string::npos) {
    return src.length() == pos + suffix.length();
  }
  return false;
}

std::string strip(std::string const& str, std::string const& whitespace) {
  auto const strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) {
    return "";  // no content
  } else {
    auto const strEnd = str.find_last_not_of(whitespace);
    auto const strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
  }
}

std::vector<std::string> split(char const delim, std::string const& str, bool const doStrip, bool const discardEmpty,
                               int const nSplits) {
  if (str.empty()) return {};

  if (nSplits == 0) return {doStrip ? strip(str) : str};

  std::vector<std::string> res;
  std::stringstream ss(str);
  std::string item;
  for (int splitCount = 0; nSplits < 0 or splitCount < nSplits; splitCount++) {
    if (not std::getline(ss, item, delim)) {
      break;
    }

    if (doStrip) {
      item = strip(item);
      if (item.empty() and delim == ' ') {
        continue;
      }
    }

    if (not item.empty() or not discardEmpty) {
      res.push_back(std::move(item));
    }
  }

  // If the stream is not fully consumed, push the rest into response
  if (std::getline(ss, item)) {
    if (doStrip) {
      item = strip(item);
    }
    res.push_back(std::move(item));
  } else {
    // otherwise we need to inspect the last element and push back an empty string
    if (str.back() == delim and not discardEmpty) {
      res.emplace_back("");
    }
  }
  return res;
}

std::vector<std::string> splitNoEmpty(char const delim, std::string const& str) {
  return split(delim, str, true, true, -1);
}

std::vector<std::string> split(char const delim, std::string const& str, bool const doStrip, int const nSplits) {
  return split(delim, str, doStrip, /*discardEmpty*/ false, nSplits);
}

// that version takes a multi-char delimiter
std::vector<std::string> splitM(char const* delim, std::string const& str) {
  std::vector<std::string> res;

  auto const pos = str.find(delim);
  if (pos == std::string::npos) {
    res.push_back(str);
  } else {
    res.push_back(str.substr(0, pos));
    res.push_back(str.substr(pos + strlen(delim)));
  }

  return res;
}

std::vector<std::string> splitByRegex(std::string const& s, char const* delim) {
  std::vector<std::string> result;
  std::regex rgx(delim);
  std::sregex_token_iterator iter(s.begin(), s.end(), rgx, -1);
  std::sregex_token_iterator end;
  for (; iter != end; ++iter) {
    result.push_back(*iter);
  }
  return result;
}

std::string lastLine(std::string const& src) {
  auto pos = src.find_last_of('\n');
  if (pos == std::string::npos) {
    return src;
  }
  return src.substr(pos + 1);
}

namespace {

class SplitIter {
 public:
  explicit SplitIter(std::string data, char separator = ',', int maxSplits = 1)
      : m_data(std::move(data)), m_separator(separator), m_maxSplits(maxSplits) {}

  std::string next() {
    std::vector<char> expectedBrackets;
    m_start_i = ++m_end_i;

    if (m_end_i >= (int)m_data.size()) {
      return "";
    }

    if (m_maxSplits >= 0 and m_splitCounter >= m_maxSplits) {
      m_end_i = (int) m_data.size();
      return m_data.substr(size_t(m_start_i));
    }

    for (; m_end_i < int(m_data.size()); m_end_i++) {
      auto const character = m_data[m_end_i];

      if (auto expectedCloseBracket = getCloseSymbol(character)) {
        expectedBrackets.push_back(expectedCloseBracket);
      } else if (isCloseSymbol(character)) {
        if (character == '>') {
          // only if we found a match we pop the bracket off and continue
          // otherwise no error will be triggered
          if (not expectedBrackets.empty() and expectedBrackets.back() == character) {
            expectedBrackets.pop_back();
          }
        } else {
          // Here if we meet > as expected we just pop them all off
          // instead of triggering an error
          while (not expectedBrackets.empty()) {
            if (expectedBrackets.back() != '>') {
              break;
            }
            expectedBrackets.pop_back();
          }

          checkCond(not expectedBrackets.empty(), "Inbalanced brackets in std::string \"" + m_data + "\".",
                    "Found closing bracket", character, "without any matching", "opening bracket.");

          auto const expectedBracket = expectedBrackets.back();
          expectedBrackets.pop_back();

          checkCond(character == expectedBracket, "Imbalanced brackets in string \"", m_data, "\".",
                    "Expected closing bracket", expectedBracket, ", but found", character);
        }
      }

      if (m_data[m_end_i] == m_separator and expectedBrackets.empty()) {
        break;
      }
    }

    checkCond(expectedBrackets.empty() or app_utils::all_of(expectedBrackets, [](auto const& x) { return x == '>'; }),
              "Imbalanced brackets in string \"", m_data, "\"." /*,
              "Expected to see the following brackets:", expectedBrackets*/);

    m_splitCounter++;
    return m_data.substr(size_t(m_start_i), size_t(m_end_i - m_start_i));
  }

  bool hasNext() const { return not m_data.empty() and m_end_i < int(m_data.size()); }

 private:
  std::string const m_data;
  char const m_separator;
  int const m_maxSplits;

  int m_start_i = -1, m_end_i = -1;
  int m_splitCounter = 0;
};
}  // namespace

std::vector<std::string> splitParse(std::string const& valStr, char delimiter, bool doStrip, int maxSplits) {
  SplitIter splitIter(valStr, delimiter, maxSplits);

  std::vector<std::string> res;
  while (splitIter.hasNext()) {
    res.emplace_back(splitIter.next());
    if (doStrip) {
      res.back() = strip(res.back());
    }
  }
  return res;
}

std::vector<std::string> splitParse(std::string const& valStr, char delimiter, bool doStrip) {
  return splitParse(valStr, delimiter, doStrip, -1);
}

std::string stripVectorBraces(std::string const& valStr) { return stripBraces(valStr); }

std::string stripBraces(std::string const& valStr, std::string const& braces) { 
  std::string out = strip(valStr); 
  if (enclosedInBraces(out)) {
    return out.substr(1, out.size() - 2);
  }
  return out;
}

std::size_t findMatchingClose(std::string const& valStr, size_t const startFrom) {
  checkCond(valStr.size() > startFrom, "inconsistent inputs");
  char openSymbol = valStr[startFrom];
  checkCond(isOpenSymbol(openSymbol), openSymbol, "is not a supported open symbol in", valStr);
  char closeSymbol = getCloseSymbol(openSymbol);
  int numOpen = 1;
  for (size_t pos = startFrom + 1; pos < valStr.size(); pos++){
    if (valStr[pos] == closeSymbol) {
      numOpen--;
    } else if (valStr[pos] == openSymbol) {
      numOpen++;
    }
    if (numOpen == 0) {
      return pos;
    }
  }
  return std::string::npos;
}

bool enclosedInBraces(std::string const& valStr, std::string const& braces) {
  checkCond(braces.size() == 2, "invalid braces specification:", braces);
  // Vector braces only exist as the first and last character.
  if (not startswith(valStr, braces[0])) {
    return false;
  }

  for (size_t unmatched_braces = 1, index = 1; index < valStr.size(); ++index) {
    if (valStr[index] == braces[0]) {
      ++unmatched_braces;

    } else if (valStr[index] == braces[1] and --unmatched_braces == 0) {
      return index == valStr.size() - 1;
    }
  }
  return false;
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

}  // namespace strutils

}  // namespace app_utils