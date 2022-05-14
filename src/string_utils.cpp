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

bool contains(std::string_view src, std::string_view substr) { return src.find(substr) != std::string::npos; }

bool contains(std::string_view src, char const c) { return src.find(c) != std::string::npos; }

bool startswith(std::string_view src, std::string_view suffix) {
  auto pos = src.find(suffix);
  return pos == 0;
}

bool startswith(std::string_view src, char const prefix) { return not src.empty() and src.front() == prefix; }

bool endswith(std::string_view src, char const suffix) { return not src.empty() and src.back() == suffix; }

bool endswith(std::string_view src, std::string_view suffix) {
  auto pos = src.rfind(suffix);
  if (pos != std::string::npos) {
    return src.length() == pos + suffix.length();
  }
  return false;
}

std::string_view strip(std::string_view const str, std::string_view const whitespace) {
  auto const strBegin = str.find_first_not_of(whitespace);
  if (strBegin == std::string::npos) {
    return "";  // no content
  } else {
    auto const strEnd = str.find_last_not_of(whitespace);
    auto const strRange = strEnd - strBegin + 1;
    return str.substr(strBegin, strRange);
  }
}

size_t split(std::string_view const str,
          char const delim, 
          std::string_view& outBuffer, 
          size_t maxNumTokens) {
  
  size_t leftIndexPos = 0;
  
  std::string_view* outputPointer = &outBuffer;

  size_t parsedTokens = 0;
  for (size_t i = 0; i < str.size(); i++) {
    if (str.at(i) == delim) {
      *outputPointer = str.substr(leftIndexPos, i - leftIndexPos);      
      leftIndexPos = i+1;      
      if (++parsedTokens >= maxNumTokens) {
        return parsedTokens;
      }
      outputPointer++;
    }
  }
  *outputPointer  = str.substr(leftIndexPos + 1);
  return ++parsedTokens;
}

std::vector<std::string_view> split(char const delim, 
                                    std::string_view const str, 
                                    bool const stripWhiteSpace,
                                    int const nSplits) {
  if (str.empty()) return {};

  if (nSplits == 0) return {stripWhiteSpace ? strip(str) : str};

  std::vector<std::string_view> res;

  size_t token_start_index = 0;
  size_t i = 0;
  for (; i < str.size(); i++) {
    char c = str[i];
    if(c == delim or (c == ' ' and stripWhiteSpace)) {
      if(i > token_start_index) {
        res.push_back(str.substr(token_start_index, i-token_start_index));
      }
      token_start_index = i + 1;
    }

    if(nSplits > 0 and (int)res.size() >= nSplits) {
      break;
    }
  }

  if (token_start_index < str.size()) {
    res.push_back(str.substr(token_start_index));
  }

  return res;
}


std::vector<std::string_view> splitNoEmpty(char const delim, std::string_view str) {
  return split(delim, str, /*strip whitespace*/true, -1);
}

// that version takes a multi-char delimiter
std::vector<std::string_view> splitM(std::string_view const delim, std::string_view const str) {
  std::vector<std::string_view> res;

  auto const pos = str.find(delim);
  if (pos == std::string::npos) {
    res.push_back(str);
  } else {
    res.push_back(str.substr(0, pos));
    res.push_back(str.substr(pos + delim.size()));
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

std::string_view lastLine(std::string_view const src) {
  auto pos = src.find_last_of('\n');
  if (pos == std::string::npos) {
    return src;
  }
  return src.substr(pos + 1);
}

namespace {

class SplitIter {
 public:
  explicit SplitIter(std::string_view data, char separator = ',', int maxSplits = 1)
      : m_data(data)
      , m_separator(separator)
      , m_maxSplits(maxSplits) {}

  std::string_view next() {
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

      if (isOpenSymbol(character)) {
        expectedBrackets.push_back(getCloseSymbol(character));

      } else if (isCloseSymbol(character)) {
        if (character != '>') {
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

          checkCond(not expectedBrackets.empty(), "Inbalanced brackets in string \"", m_data, "\".",
                    "Found closing bracket", character, "without any matching opening bracket.");

          auto const expectedBracket = expectedBrackets.back();
          expectedBrackets.pop_back();

          checkCond(character == expectedBracket, "Imbalanced brackets in string \"", m_data, "\".",
                    "Expected closing bracket", expectedBracket, ", but found", character);
        }
      } else if (character == m_separator) {
        if (expectedBrackets.empty()) {
          break;
        }
      }
    }

    checkCond(expectedBrackets.empty() or app_utils::all_of(expectedBrackets, [](auto const& x) { return x == '>'; }),
              "Imbalanced brackets in string \"", m_data, "\"." /*,
              "Expected to see the following brackets:", expectedBrackets*/);

    m_splitCounter++;
    return m_data.substr(size_t(m_start_i), size_t(m_end_i - m_start_i));
  }

  [[nodiscard]]
  bool hasNext() const { return not m_data.empty() and m_end_i < int(m_data.size()); }

 private:
  std::string_view const m_data;
  char const m_separator;
  int const m_maxSplits;

  int m_start_i = -1, m_end_i = -1;
  int m_splitCounter = 0;
};
}  // namespace

std::vector<std::string_view> splitParse(std::string_view const valStr,
                                         char delimiter,
                                         bool doStrip,
                                         int maxSplits) {
  SplitIter splitIter(valStr, delimiter, maxSplits);

  std::vector<std::string_view> res;
  while (splitIter.hasNext()) {
    res.emplace_back(splitIter.next());
    if (doStrip) {
      res.back() = strip(res.back());
    }
  }
  return res;
}

std::vector<std::string_view> splitParse(std::string_view const valStr, char delimiter, bool doStrip) {
  return splitParse(valStr, delimiter, doStrip, -1);
}

std::string_view stripVectorBraces(std::string_view const valStr) { return stripBraces(valStr); }

std::string_view stripBraces(std::string_view const valStr, std::string_view const braces) { 
  std::string_view out = strip(valStr); 
  if (enclosedInBraces(out, braces)) {
    return out.substr(1, out.size() - 2);
  }
  return out;
}

std::size_t findMatchingClose(std::string_view const valStr, size_t const startFrom) {
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

bool enclosedInBraces(std::string_view const valStr, std::string_view const braces) {
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

void replaceAll(std::string& str, std::string_view const from, std::string_view const to) {
  if (from.empty()) return;
  size_t start_pos = 0;
  while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
    str.replace(start_pos, from.length(), to);
    start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
  }
}

}  // namespace strutils

}  // namespace app_utils