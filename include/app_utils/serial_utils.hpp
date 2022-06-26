#pragma once

#include <cstddef> // std::byte
#include "serial_type_utils.hpp"

namespace app_utils::serial {

template<typename T1, typename T2, typename ...T>
size_t from_bytes(std::byte const* data, size_t len, T1& val1, T2& val2, T&... val) {
  size_t num_read = from_bytes(data, len, val1);
  num_read += from_bytes(data + num_read, len - num_read, val2);
  ((num_read += from_bytes(data + num_read, len - num_read, val)), ...);
  return num_read;
}

template<typename T1, typename T2, typename ...T>
size_t to_bytes(std::byte* data, size_t len, T1 const& val1, T2 const& val2, T const&... val) {
  size_t num_written = to_bytes(data, len, val1);
  num_written += to_bytes(data + num_written, len - num_written, val2);
  ((num_written += to_bytes(data + num_written, len - num_written, val)), ...);
  return num_written;
}

template<typename T>
size_t from_bytes(std::span<std::byte const> bytes, T& val) {
  return from_bytes(bytes.data(), bytes.size(), val);
}

template<typename T>
size_t to_bytes(std::vector<std::byte>& bytes, T const& val) {
  bytes.resize(serial_size(val));
  return to_bytes(bytes.data(), bytes.size(), val);
}

}  // namespace app_utils::serial
