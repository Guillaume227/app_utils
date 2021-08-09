#pragma once

#include <app_utils/rtti_check.hpp>

#ifdef RTTI_ENABLED
#include <app_utils/cond_check.hpp>
#else
#define throwExc(...)
#endif

#include <string_view>
#include <array>

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

struct enum_value_detail_t {
  std::string_view value_name;
  int int_value;
};

inline void compile_time_error(std::string_view /*message*/) {}

constexpr int stoi(std::string_view const str) {
  bool has_negative_sign = false;
  int value = 0;
  auto const is_digit = [](char c) { return c <= '9' and c >= '0'; };

  for (char c : str) {
    if (c == '-') {
      has_negative_sign = true;
    } else {
      if (not is_digit(c)) {
        compile_time_error("not a digit");
      }
      value = value * 10 + (c - '0');
    }
  }
  if (has_negative_sign) {
    value *= -1;
  }
  return value;
}

/* list of all enum values as strings */
template<size_t N>
consteval std::array<enum_value_detail_t, N> parseEnumDefinition(std::string_view const enum_vals_as_str) {

  std::array<enum_value_detail_t, N> value_details;

  size_t token_index = 0;
  size_t left_index = 0;
  size_t num_chars = 0;
  bool is_parsing_name = true; // strip out explicit value specialization ( e.g. "val1 = 0, ...")  

  auto const assign_detail = [&]() {
    auto token_str = enum_vals_as_str.substr(left_index, num_chars);
    if (is_parsing_name) {
      value_details[token_index].value_name = token_str;
    } else {     
      value_details[token_index].int_value = stoi(token_str);
    }
  };
  for (size_t i = 0; i < enum_vals_as_str.size(); i++) {

    switch (char c = enum_vals_as_str[i]) {
    case ' ':
    case '\n':
    case ',':
    case '=':
      if (num_chars > 0) {
        assign_detail();        
        num_chars = 0;
      }
      if (c == ',') {
        if (is_parsing_name) {
          value_details[token_index].int_value = token_index == 0
              ? 0
              : value_details [token_index - 1].int_value + 1;
        } else {
          is_parsing_name = true;
        }
        token_index++;
      } else if (c == '=') {
        is_parsing_name = false;
      }
      left_index = i + 1;
      break;
    default:
      num_chars++;
      break;
    }
  }

  if (num_chars > 0) {
    assign_detail();
    if (is_parsing_name) {
      value_details[token_index].int_value = token_index == 0
          ? 0
          : value_details[token_index - 1].int_value + 1;
    } 
  }

  return value_details;
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
  constexpr static auto enum_value_details =
      enumatic::parseEnumDefinition<size()>(enumValuesAsString(EnumType{}));

  consteval static bool has_default_indexation() {
    for (int i = 0; i < (int) enum_value_details.size(); i++) {
      if (enum_value_details[i].int_value != i) {
        return false;
      }
    }
    return true;
  }

  consteval static auto getValues() { 
    std::array<EnumType, size()> values;
    for (size_t i = 0; i < size(); i++) {
      values[i] = static_cast<EnumType>(enum_value_details[i].int_value);
    }
    return values;
  }

  /* To/from std::string conversion */
  constexpr static std::string_view toString(EnumType arg) {
    if constexpr (has_default_indexation()) {
      return enum_value_details[static_cast<size_t>(arg)].value_name;
    } else {
      for (auto const& value_detail : enum_value_details) {
        if (value_detail.int_value == static_cast<int>(arg))
          return value_detail.value_name;
      }
      return "";// should never get there - compile time error
    }
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

    for (auto const& value_detail : enum_value_details) {
      if (val == value_detail.value_name) {
        enumVal = static_cast<EnumType>(value_detail.int_value);
        return true;
      }
    }

    if constexpr (allowConvertFromIndex(EnumType{})) {
      bool hasOnlyDigits = val.find_first_not_of("-0123456789") == std::string_view::npos;
      if (hasOnlyDigits) {
        int intVal = enumatic::stoi(val);
        for (auto const& value_detail : enum_value_details) {
          if (value_detail.int_val == intVal) {
            enumVal = static_cast<EnumType>(intVal);
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
    consteval std::string_view enumValuesAsString(EnumType) { return #__VA_ARGS__; }                               \
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
