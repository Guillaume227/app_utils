#include <app_utils/stream_utils.hpp>
#include <array>

#if defined(__GNUG__) && !defined(__clang__)
#include <cxxabi.h>
#endif

namespace app_utils {

std::string_view parseTypeName(std::string_view paramName, bool const minimal) {
#if defined(__GNUG__) && !defined(__clang__)
  int status;
  char* demangled = abi::__cxa_demangle(paramName.data(), 0, 0, &status);
  if (!status)
    paramName = demangled;
    //free(demangled);
#endif

  // remove "class" prefix
  std::array<std::string_view, 2> const prefixes = {"class", "struct"};
  for (auto const& prefix: prefixes) {
    if (paramName.find(prefix) == 0) {
      paramName = paramName.substr(prefix.size() + 1);
    }
  }

  // remove trailing "const *" info etc. suffix
  auto const templateBracketPos = paramName.rfind('>');
  auto const starPos = paramName.find(' ');
  // check space is beyond the last > as otherwise template types may not display correctly
  if (starPos != std::string_view::npos && (templateBracketPos == std::string_view::npos || starPos > templateBracketPos))
    paramName = paramName.substr(0, starPos);

  // suppress digits from start of string to get the real class name
  // without the platform specific mangling
  size_t i = 0;
  for (; i < paramName.size(); i++) {
    if (!isdigit(paramName[i]))
      break;
  }
  if (i > 0)
    paramName = paramName.substr(i);

  if (minimal) {
    paramName = paramName.substr(paramName.rfind(':') + 1);
  }
  return paramName;
}
}// namespace app_utils
