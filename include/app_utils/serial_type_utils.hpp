#pragma once

#include <cstddef> // std::byte
#include <cstring> // std::memcpy
#include <type_traits>
#include <string>
#include <vector>
#include <bitset>
#include <array>
#include <span>
#include <bit>

namespace app_utils::serial {

static_assert(std::endian::native == std::endian::little, "serialization logic assumes little-endian format");

/**
 * serial_size(...) exists in different flavors:
 *  - a set of overloads that take a pointer, so that ADL can be used for custom types that do not require actual instance details
 *      as that pointer can be nullptr.
 *  - a set of overloads that take a const& for types whose serialization size depend on the instance (e.g. vector, string, string_view)
 */
template<typename T>
  requires (std::is_trivially_copyable_v<T> and not std::is_same_v<T, char>)
constexpr size_t serial_size(T const*) {
  return sizeof(T);
}

constexpr size_t serial_size(char const&) {
  return 1;
}

template<size_t N>
constexpr size_t serial_size(char const(&) [N]) {
  return N;
}

template<typename T, size_t N>
constexpr size_t serial_size(std::array<T, N> const*) {
  if constexpr(std::is_same_v<T, char>) {
    return N;
  } else {
    return N * serial_size((T const*) nullptr);
  }
}

template <size_t N>
constexpr size_t serial_size(std::bitset<N> const*) {
  return (N + 7) / 8; // round up to the nearest number of bytes // == sizeof(val) ?
}

template<typename T>
  requires (not std::is_pointer_v<T>)
constexpr size_t serial_size(T const& val) {
  return serial_size(&val);
}

template <typename T>
constexpr size_t serial_size(std::vector<T> const& val) {
  size_t num_bytes = 1; // 1 byte for holding the size
  for (auto& item : val) {
    num_bytes += serial_size(item);
  }
  return num_bytes;
}

inline size_t serial_size(std::string const& str) {
  return 1 + str.size(); // 1 extra byte for holding the size
}

/**
 * std::string_view
 * note that it doesn't store the size, unlike the std::string specialization.
 * It's the same behavior as char[]
 */
constexpr size_t serial_size(std::string_view const& val) {
  return val.size();
}

// Note: unlike for std::vector, serialization of a span
// doesn't keep track of the size - rationale is that you can't deserialize a span
// as it's a view type, so only raw contents gets serialized.
template <typename T>
constexpr size_t serial_size(std::span<T> const& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += serial_size(item);
  }
  return num_bytes;
}

/**
 *  arithmetic types
 */
template<typename T>
  requires (std::is_trivially_copyable_v<T> and not std::is_enum_v<T>)
constexpr size_t from_bytes(std::byte const* buffer, size_t /*buffer_size*/, T& val) {
  size_t num_bytes = serial_size(val);
#if defined(__GNUC__)  && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wclass-memaccess"
#endif
  std::memcpy(&val, buffer, num_bytes);
#if defined(__GNUC__)  && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
  return num_bytes;
}

template <typename T>
  requires (std::is_trivially_copyable_v<T> and not std::is_enum_v<T>)
constexpr size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, T const& val) {
  size_t num_bytes = serial_size(val);
  std::memcpy(buffer, &val, num_bytes);
  return num_bytes;
}

/**
 * bool
 */
constexpr size_t from_bytes(std::byte const* buffer, size_t /*buffer_size*/, bool& val) {
  val = std::to_integer<uint8_t>(buffer[0]) != 0;
  return 1;
}

constexpr size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, bool const& val) {
  buffer[0] = static_cast<std::byte>(val);
  return 1;
}

/**
 * enum
 */
template <typename T>
  requires std::is_enum_v<T>
constexpr size_t from_bytes(std::byte const* buffer, size_t buffer_size, T& val) {
  constexpr size_t num_bytes = serial_size(T{});
  if constexpr (num_bytes == 1) {
    val = static_cast<T>(std::to_integer<std::underlying_type_t<T>>(buffer[0]));
  } else {
    std::underlying_type_t<T> val_int;
    from_bytes(buffer, buffer_size, val_int);
    val = static_cast<T>(val_int);
  }
  return num_bytes;
}

template <typename T>
  requires std::is_enum_v<T>
constexpr size_t to_bytes(std::byte* buffer, size_t buffer_size, T const& val) {
  constexpr size_t num_bytes = serial_size(T{});
  if constexpr (num_bytes == 1) {
    buffer[0] = static_cast<std::byte>(val);
  } else {  
    to_bytes(buffer, buffer_size, static_cast<std::underlying_type_t<T>>(val));
  }
  return num_bytes;
}

/**
 * std::string
 * NOTE: size cannot exceed 256
 */
inline size_t from_bytes(std::byte const* buffer, size_t /*buffer_size*/, std::string& val) {
  size_t str_size = std::to_integer<size_t>(buffer[0]);
  val.resize(str_size);
  std::memcpy(val.data(), buffer+1, str_size);
  return str_size + 1;
}

inline size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, std::string const& val) {
  size_t str_size = val.size();
  buffer[0] = static_cast<std::byte>(str_size);
  std::memcpy(buffer+1, val.c_str(), str_size);
  return str_size + 1;
}

/**
 * std::string_view
 * note that it doesn't store the size, unlike the std::string specialization
 */
inline size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, std::string_view const& val) {
  size_t str_size = val.size();
  std::memcpy(buffer, val.data(), str_size);
  return str_size;
}
template<size_t N>
constexpr size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, char const (& val) [N]) {
  std::memcpy(buffer, val, N);
  return N;
}

template<size_t N>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, std::bitset<N>& val) {
  // note: for backward compatibility, allow a serial size smaller than buffer size.
  size_t num_bytes = std::min(serial_size(val), buffer_size);
  val.reset();
  for (size_t i = 0; i < N; i++) {
    if (i/8 >= num_bytes) {
      break;
    }
    auto b = std::byte(1 << i % 8);
    if ((buffer[i/8] & b) == b) {
      val.set(i);
    }
  }
  return num_bytes;
}

template<size_t N>
size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, std::bitset<N> const& val) {
  size_t num_bytes = serial_size(val);
  std::memset(buffer, 0, num_bytes);
  for (size_t i = 0; i < N; i++) {
    if (val.test(i)) {
      buffer[i/8] |= std::byte(1 << i % 8);
    }
  }
  return num_bytes;
}


/**
 * C-style array
 */
template <typename T, size_t N>
  requires std::is_arithmetic_v<T>
constexpr size_t from_bytes(std::byte const* buffer, size_t /*buffer_size*/, T (&val)[N]) {
  std::memcpy(val, buffer, N);
  return 1;
}

template <typename T, size_t N>
  requires std::is_arithmetic_v<T>
constexpr size_t to_bytes(std::byte* buffer, size_t /*buffer_size*/, T const (&val) [N]) {
  std::memcpy(buffer, val, N);
  return N;
}

/**
 * std::array
 */

template <typename T, size_t N>
constexpr size_t from_bytes(std::byte const* buffer, size_t buffer_size, std::array<T, N>& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += from_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

template <typename T, size_t N>
constexpr size_t to_bytes(std::byte* const buffer, size_t const buffer_size, std::array<T, N> const& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += to_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

/**
 * std::vector
 */

template<typename T>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, std::vector<T>& val) {
  uint8_t str_size = std::to_integer<uint8_t>(buffer[0]);
  val.resize(str_size);
  size_t num_bytes = 1;
  for (auto& item : val) {
    num_bytes += from_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

template <typename T>
size_t to_bytes(std::byte* buffer, size_t buffer_size, std::vector<T> const& val) {
  size_t str_size = val.size();
  buffer[0] = static_cast<std::byte>(str_size);
  size_t num_bytes = 1;
  for (auto& item : val) {
    num_bytes += to_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

template <typename T>
size_t to_bytes(std::byte* buffer, size_t buffer_size, std::span<T> const& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += to_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}
}  // namespace app_utils::serial
