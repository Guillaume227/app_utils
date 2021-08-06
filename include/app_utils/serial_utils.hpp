#pragma once

#include <app_utils/cond_check.hpp>
#include <cstddef> // std::byte
#include <type_traits>

#include <string>
#include <vector>
#include <array>

namespace app_utils::serial {

template<typename T>
constexpr size_t serial_size(T const&) requires std::is_arithmetic_v<T> or std::is_enum_v<T> {
  return sizeof(T);
}

constexpr size_t serial_size(std::string const& str) {
  return 1 + str.size();
}

template<typename T, size_t N>
constexpr size_t serial_size(std::array<T, N> const&) { 
  return N * sizeof(T);
}

template <typename T>
constexpr size_t serial_size(std::vector<T> const& val) {
  return 1 + val.size() * sizeof(T);
}

template<typename T>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, T& val) requires std::is_arithmetic_v<T> {
  size_t num_bytes = serial_size(val);
  // TODO: endianness
  std::memcpy(&val, buffer, num_bytes);
  return num_bytes;
}

template <typename T>
size_t to_bytes(std::byte* buffer, size_t buffer_size, T const& val) requires std::is_arithmetic_v<T> {
  size_t num_bytes = serial_size(val);
  // TODO: endianness
  std::memcpy(buffer, &val, num_bytes);
  return num_bytes;
}

size_t from_bytes(std::byte const* buffer, size_t buffer_size, bool& val) {
  val = std::to_integer<uint8_t>(buffer[0]) != 0;
  return 1;
}

size_t to_bytes(std::byte* buffer, size_t buffer_size, bool const& val) {
  buffer[0] = static_cast<std::byte>(val);
  return 1;
}

template <typename T>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, T& val) requires std::is_enum_v<T> {
  size_t num_bytes = serial_size(val);
  std::memcpy(&val, buffer, num_bytes);
  return num_bytes;
}

template <typename T>
size_t to_bytes(std::byte* buffer, size_t buffer_size, T const& val) requires std::is_enum_v<T> {
  size_t num_bytes = serial_size(val);
  std::memcpy(buffer, &val, num_bytes);
  return num_bytes;
}

size_t from_bytes(std::byte const* buffer, size_t buffer_size, std::string& val) {
  size_t str_size = std::to_integer<size_t>(buffer[0]);
  val.resize(str_size);
  std::memcpy(val.data(), buffer+1, str_size);
  return str_size + 1;
}

size_t to_bytes(std::byte* buffer, size_t buffer_size, std::string const& val) {  
  size_t str_size = val.size();
  buffer[0] = static_cast<std::byte>(str_size);
  std::memcpy(buffer+1, val.c_str(), str_size);
  return str_size + 1;
}

template <typename T, int N>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, T (&val)[N]) requires std::is_arithmetic_v<T> {
  std::memcpy(val, buffer, N);
  return 1;
}

template <typename T, int N>
size_t to_bytes(std::byte* buffer, size_t buffer_size, T const (&val) [N]) requires std::is_arithmetic_v<T> {
  std::memcpy(buffer, val, N);
  return N;
}

template <typename T, int N>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, std::array<T, N>& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += from_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

template <typename T, int N>
size_t to_bytes(std::byte* buffer, size_t buffer_size, std::array<T, N> const& val) {
  size_t num_bytes = 0;
  for (auto& item : val) {
    num_bytes += to_bytes(buffer + num_bytes, buffer_size - num_bytes, item);
  }
  return num_bytes;
}

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
}  // namespace app_utils::serial