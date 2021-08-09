#pragma once

#include <app_utils/rtti_check.hpp>

#ifdef RTTI_ENABLED
#include <app_utils/cond_check.hpp>
#else
#define throwExc(...)
#endif

#include <string_view>
#include <array>
#include <charconv>

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

/* list of all enum values as strings */
template<size_t N>
consteval std::array<std::string_view, N> parseEnumDefinition(std::string_view const enum_vals_as_str) {

  std::array<std::string_view, N> values;

  size_t token_found = 0;
  size_t left_index = 0;
  size_t num_chars = 0;
  bool new_val_can_start = true; // strip out explicit value specialization ( e.g. "val1 = 0, ...")  
  for (size_t i = 0; i < enum_vals_as_str.size(); i++) {

    switch (char c = enum_vals_as_str[i]) {
    case ' ':
    case '\n':
    case ',':
    case '=':
      if (num_chars > 0) {
        values[token_found++] = enum_vals_as_str.substr(left_index, num_chars);
        num_chars = 0;
        new_val_can_start = false;
      }
      if (c == ',') {
        new_val_can_start = true;
      }
      left_index = i + 1;
      break;
    default:
      if (new_val_can_start) {
        num_chars++;
      }
      break;
    }
  }

  if (num_chars > 0) {
    values[token_found++] = enum_vals_as_str.substr(left_index, num_chars);
  }

  return values;
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
  return is_enumatic_type((T*)nullptr);
}
}  // namespace enumatic

/*Enum companion class to hold the methods that can't be declared in an enum */
template <typename EnumType>
struct Enumatic {
  Enumatic() = delete; /* prevents instantiation */
  constexpr static std::string_view name() { return typeName(EnumType{} /*dummy value - only type matters here for ADL*/); }

  consteval static size_t size() {
    return enumatic::numListItems(enumValuesAsString(EnumType{} /*dummy value - only type matters here for ADL*/));
  }

  /* list of all enum values as strings */
  constexpr static std::array<std::string_view, size()> enum_value_details = enumatic::parseEnumDefinition<size()>(enumValuesAsString(EnumType{}));

  consteval static auto getValues() { return make_array(std::make_index_sequence<size()>()); }

  /* To/from std::string conversion */
  constexpr static std::string_view toString(EnumType arg) {    
    return enum_value_details[static_cast<unsigned>(arg)];
  }

  /* returns true if conversion was successfull */
  constexpr static bool fromString(std::string_view val, EnumType& enumVal) {
    
    if (val.empty()) {
      return false;
    }

    // strip out enum name from value. 
    // '.' can be used as a separator in a python binding
    for (std::string_view separator : {"::", "."}) {            
      if (val.starts_with(name()) and val.substr(name().size()).starts_with(separator)) {
        val.remove_prefix(name().size() + separator.size());
      }      
    }

    for (size_t i = 0; i < size(); i++) {
      if (val == enum_value_details[i]) {
        enumVal = getValues()[i];
        return true;
      }
    }

    if constexpr (allowConvertFromIndex(EnumType{})) {
      bool hasOnlyDigits = val.find_first_not_of("-0123456789") == std::string_view::npos;
      if (hasOnlyDigits) {
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
    }
    return false;
  }

  /* Attempt at converting from std::string avlue - throws on failure */
  constexpr static EnumType fromString(std::string_view const val) {
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

/*
* Regarding the use of ADL in the wrapper class containing the actual enum definition:
 For arguments of enumeration type, the innermost enclosing namespace 
 of the declaration of the enumeration type is defined is added to the set. 
 If the enumeration type is a member of a class, that class is added to the set.
 */

#define ENUMATIC_DEFINE_IMPL(EnumClass, StorageType, EnumName, allowFromIdx, ...)                                  \
  namespace EnumName##_wrapper_t {                                                                                 \
    EnumClass EnumType StorageType {__VA_ARGS__};                                                                  \
    consteval std::string_view typeName(EnumType) { return #EnumName; }                                            \
    consteval std::string_view enumValuesAsString(EnumType) { return #__VA_ARGS__; }                                    \
    consteval size_t size(EnumType) { return enumatic::numListItems(#__VA_ARGS__); }                               \
    constexpr std::string_view to_string(EnumType arg) { return Enumatic<EnumType>::toString(arg); }               \
                                                                                                                   \
    constexpr bool is_enumatic_type(EnumType*) noexcept { return true; }                                           \
                                                                                                                   \
    consteval bool allowConvertFromIndex(EnumType) { return allowFromIdx; }                                        \
                                                                                                                   \
    constexpr size_t serial_size(EnumType) {                                                                       \
      if constexpr (size(EnumType{}) > 255) {                                                                      \
        return sizeof(EnumType);                                                                                   \
      } else {                                                                                                     \
        return 1;                                                                                                  \
      }                                                                                                            \
    }                                                                                                              \
  };                                                                                                               \
                                                                                                                   \
  using EnumName = EnumName##_wrapper_t::EnumType

#define ENUM_CLASS_DEFINE(EnumName, fromIdx, ...) ENUMATIC_DEFINE_IMPL(enum class, , EnumName, fromIdx, __VA_ARGS__)
#define ENUM_DEFINE(EnumName, ...) ENUMATIC_DEFINE_IMPL(enum, , EnumName, false, __VA_ARGS__)

#define ENUMATIC_DEFINE(EnumName, ...) ENUM_CLASS_DEFINE(EnumName, false, __VA_ARGS__)
#define ENUMATIC_DEFINE_idx(EnumName, ...) ENUM_CLASS_DEFINE(EnumName, true, __VA_ARGS__)
