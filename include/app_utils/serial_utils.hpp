#pragma once

#include <app_utils/cond_check.hpp>
#include <string>
#include <cstddef> // std::byte

template<typename T>
constexpr size_t serial_size(T const&) {
  return sizeof(T);
}

template <>
constexpr size_t serial_size<std::string>(std::string const& str) {
  return 1 + str.size();
}


template<typename T>
size_t from_bytes(std::byte const* buffer, size_t buffer_size, T& val) {
  size_t num_bytes = serial_size(val);
  // TODO: endianness
  std::memcpy(&val, buffer, num_bytes);
  return num_bytes;
}

template <typename T>
size_t to_bytes(std::byte* buffer, size_t buffer_size, T const& val) {
  size_t num_bytes = serial_size(val);
  // TODO: endianness
  std::memcpy(buffer, &val, num_bytes);
  return num_bytes;
}


template <>
size_t from_bytes<std::string>(std::byte const* buffer, size_t buffer_size, std::string& val) {
  uint8_t str_size = std::to_integer<uint8_t>(buffer[0]);
  val.resize(str_size);
  // TODO: endianness
  std::memcpy(val.data(), buffer+1, str_size);
  return str_size + 1;
}

template <>
size_t to_bytes<std::string>(std::byte* buffer, size_t buffer_size, std::string const& val) {  
  size_t str_size = val.size();  
  // TODO: endianness
  buffer[0] = static_cast<std::byte>(str_size);
  std::memcpy(buffer+1, val.c_str(), str_size);
  return str_size + 1;
}


