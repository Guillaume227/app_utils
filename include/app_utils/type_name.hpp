#pragma once

#include <string_view>
#include <string>
#include <typeinfo>
#include <array>

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

template<typename T, size_t N>
std::string_view typeName(std::array<T, N> const& t, bool minimal = false) {
  static std::string const type_name =
          "std::array<" + std::string{typeName<T>(t[0], minimal)} + ", " + std::to_string(N) + ">";
  return type_name;
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
