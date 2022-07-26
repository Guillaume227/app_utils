#pragma once

#include <string_view>

namespace app_utils {
std::string_view parseTypeName(std::string_view paramName, bool minimal = false);

template<typename T>
std::string_view typeName(bool minimal = false) {
  return parseTypeName(typeid(T).name(), minimal);
}

template<typename T>
std::string_view typeName(T const& t, bool minimal = false) {
  return parseTypeName(typeid(t).name(), minimal);
}
}// namespace app_utils
