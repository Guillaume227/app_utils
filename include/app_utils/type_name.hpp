#pragma once

#include <string_view>
#include <typeinfo>

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

template<>
constexpr std::string_view typeName<uint8_t>(bool) {
  return "uint8_t";
}
template<>
constexpr std::string_view typeName<int8_t>(bool) {
  return "int8_t";
}
template<>
constexpr std::string_view typeName<uint16_t>(bool) {
  return "uint16_t";
}
template<>
constexpr std::string_view typeName<int16_t>(bool) {
  return "int16_t";
}
template<>
constexpr std::string_view typeName<uint32_t>(bool) {
  return "uint32_t";
}
template<>
constexpr std::string_view typeName<int32_t>(bool) {
  return "int32_t";
}


template<>
constexpr std::string_view typeName(uint8_t const&, bool) {
  return typeName<uint8_t>();
}
template<>
constexpr std::string_view typeName(int8_t const&, bool) {
  return typeName<int8_t>();
}
template<>
constexpr std::string_view typeName(uint16_t const&, bool) {
  return typeName<uint16_t>();
}
template<>
constexpr std::string_view typeName(int16_t const&, bool) {
  return typeName<int16_t>();
}
template<>
constexpr std::string_view typeName(uint32_t const&, bool) {
  return typeName<uint32_t>();
}
template<>
constexpr std::string_view typeName(int32_t const&, bool) {
  return typeName<int32_t>();
}

}// namespace app_utils
