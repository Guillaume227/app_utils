#pragma once

#include <app_utils/rtti_check.hpp>

#ifndef RTTI_ENABLED
#define ENUMATIC_DEFINE(EnumName, ...) enum EnumName { __VA_ARGS__ }

#else

#include <string>
#include <array>
#include <charconv>

#include <app_utils/string_utils.hpp>
#include <app_utils/cond_check.hpp>

namespace enumatic {

template <typename T>
using isEnum = typename std::enable_if<std::is_enum<T>::value>::type;

constexpr size_t numListItems(std::string_view const valStr) {
  size_t num_comas = 0;
  size_t i = 0;
  for (; i < valStr.size(); i++) {
    if (valStr[i] == ',') {
      num_comas++;
    }
  }

  if (valStr[i - 1] == ',') {
    // avoid an extra comma scenario where input __VA_ARGS__ looks like 'a, b, c,'
    // which is a legal declaration in enum
    return num_comas;
  } else {
    return num_comas + 1;
  }
}

template <typename EnumClass>
constexpr bool allowConvertFromIndex(EnumClass) {
  return false;
}

template <typename T>
constexpr bool is_enumatic_type(T*) noexcept {
  return false;
}

template <typename T>
constexpr bool is_enumatic_type() noexcept {
  return is_enumatic_type<T>(nullptr);
}
}  // namespace enumatic

/*Enum companion class to hold the methods that can't be declared in an enum */
template <typename EnumType>
struct Enumatic {
  Enumatic() = delete; /* prevents instantiation */
  constexpr static std::string_view name() { return typeName(EnumType{} /*dummy value - only type matters here for ADL*/); }

  constexpr static size_t size() {
    return enumatic::numListItems(enumValuesAsString(EnumType{} /*dummy value - only type matters here for ADL*/));
  }

  /* list of all enum values as strings */
  static std::array<std::string_view, size()> const& getValuesStr() {
    static std::array<std::string_view, size()> const valuesStr = [] {
      std::array<std::string_view, size()> values;
      app_utils::strutils::split(
        enumValuesAsString(EnumType{} /*dummy value - only type matters here for ADL*/),
        ',', 
        values.front(), 
        values.size());
      
      for (auto& value : values) {        
        auto pos = value.find('=');
        // strip out explicit enum value specification
        if (pos != std::string::npos) {
          value = value.substr(0, pos);
        }
        value = app_utils::strutils::strip(value);
      }
      return values;
    }();
    return valuesStr;
  }

  constexpr static auto getValues() { return make_array(std::make_index_sequence<size()>()); }

  /* To/from std::string conversion */
  constexpr static std::string_view toString(EnumType arg) { return getValuesStr().at(static_cast<unsigned>(arg)); }

  /* returns true if conversion was successfull */
  static bool fromString(std::string_view val, EnumType& enumVal) {
    // for legacy reasons we sometimes want case-insensitivity.

    if (val.empty()) {
      return false;
    }

    auto const& strValues = getValuesStr();

    // strip out enum name from value. 
    // '.' can be used as a separator in a python binding
    for (auto const& separator : {"::", "."}) {
      if (app_utils::strutils::contains(val, separator)) {
        std::string const enum_typeNamePrefix = std::string{name()} + separator;
        if (app_utils::strutils::startswith(val, enum_typeNamePrefix)) {
          val = val.substr(enum_typeNamePrefix.size());
        }
      }
    }

    for (size_t i = 0; i < strValues.size(); i++) {
      if (val == strValues[i]) {
        enumVal = getValues()[i];
        return true;
      }
    }

    using namespace enumatic;
    if (allowConvertFromIndex(enumVal) and app_utils::strutils::hasOnlyDigits(val)) {
      int intVal;
      auto result = std::from_chars(val.data(), val.data() + val.size(), intVal);
      if (result.ec == std::errc::invalid_argument) {
        return false;
      }
      for (auto value : getValues()) {
        if (static_cast<int>(value) == intVal) {
          enumVal = value;
          return true;
        }
      }
    }
    return false;
  }

  /* Attempt at converting from std::string avlue - throws on failure */
  static EnumType fromString(std::string_view const val) {
    EnumType enumVal;
    if (not fromString(val, enumVal)) {
      throwExc(" '", val, "' is not a valid '", name(), "' value. Options are:", enumValuesAsString(EnumType{}));
    }
    return enumVal;
  }

 private:
  /* Helper method to initialize array of enum values */
  template <std::size_t... Idx>
  constexpr static auto make_array(std::index_sequence<Idx...>) {
    return std::array<EnumType, size()>{{static_cast<EnumType>(Idx)...}};
  }
};

namespace pybind11 {
template <typename Type>
class enum_;

template <typename EnumaticT, typename EnumParent>
void wrap_enumatic(EnumParent& pymodule) {
  auto wrappedEnum = enum_<EnumaticT>(pymodule, typeName(EnumaticT{}).data());
  for (auto const& value : Enumatic<EnumaticT>::getValues()) {
    wrappedEnum.value(Enumatic<EnumaticT>::toString(value).data(), value);
  }
}
}  // namespace pybind11

#define ENUMATIC_DEFINE_IMPL(EnumClass, StorageType, EnumName, allowFromIdx, ...)                                  \
  struct EnumName##Wrapper {                                                                                       \
    EnumClass EnumType StorageType;                                                                                \
                                                                                                                   \
    friend constexpr std::string_view typeName(EnumType) { return #EnumName; }                                     \
    friend constexpr std::string_view enumValuesAsString(EnumType) { return #__VA_ARGS__; }                        \
    friend constexpr size_t size(EnumType) {                                                                       \
      /* Trisck to get the number of enum members: */                                                              \
      /*dump all the enum values in an array and compute its size */                                               \
      enum Temp { __VA_ARGS__ };                                                                                   \
      Temp enumArray[]{__VA_ARGS__};                                                                               \
      return sizeof(enumArray) / sizeof(EnumType);                                                                 \
    }                                                                                                              \
    friend constexpr bool is_enumatic_type(EnumType*) noexcept { return true; }                                    \
                                                                                                                   \
    friend std::string to_string(EnumType arg) { return std::string{Enumatic<EnumType>::toString(arg)}; }          \
                                                                                                                   \
    friend std::ostream& operator<<(std::ostream& output, EnumType const& arg) {                                   \
      return output << Enumatic<EnumType>::toString(arg);                                                          \
    }                                                                                                              \
                                                                                                                   \
    friend void from_string(std::string_view valStr, EnumType& arg) {                                              \
      arg = Enumatic<EnumType>::fromString(valStr);                                                                \
    }                                                                                                              \
                                                                                                                   \
    friend std::istream& operator>>(std::istream& input, EnumType& arg) {                                          \
      std::string valStr;                                                                                          \
      input >> valStr;                                                                                             \
      from_string(valStr, arg);                                                                                    \
      return input;                                                                                                \
    }                                                                                                              \
                                                                                                                   \
    friend constexpr bool allowConvertFRomIndex(EnumType) { return allowFromIdx; }                                 \
                                                                                                                   \
    EnumClass EnumType StorageType {__VA_ARGS__};                                                                 \
  };                                                                                                               \
                                                                                                                   \
  using EnumName = EnumName##Wrapper::EnumType

#define ENUM_CLASS_DEFINE(EnumName, fromIdx, ...) ENUMATIC_DEFINE_IMPL(enum class, , EnumName, fromIdx, __VA_ARGS__)
#define ENUM_DEFINE(EnumName, ...) ENUMATIC_DEFINE_IMPL(enum, , EnumName, false, __VA_ARGS__)

#define ENUMATIC_DEFINE(EnumName, ...) ENUM_CLASS_DEFINE(EnumName, false, __VA_ARGS__)
#define ENUMATIC_DEFINE_idx(EnumName, ...) ENUM_CLASS_DEFINE(EnumName, true, __VA_ARGS__)

#endif // RTTI_ENABLED