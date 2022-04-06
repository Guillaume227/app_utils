#pragma once

#include <cstddef> // std::byte
#include <span>

// to be included after serial_utils.hpp

namespace app_utils::serial {

template<typename ...Args>
constexpr size_t to_bytes(std::span<std::byte> buffer, Args const& ... args) {
  std::byte * const buffer_ptr = buffer.data();
  size_t read_bytes = 0;
  ((read_bytes += to_bytes(buffer_ptr + read_bytes, buffer.size() - read_bytes, args)), ...);
  return read_bytes;
}

template<typename ...Args>
size_t from_bytes(std::span<std::byte> const buffer, Args& ... args) {
  std::byte const* const buffer_ptr = buffer.data();
  size_t read_bytes = 0;
  ((read_bytes += from_bytes(buffer_ptr + read_bytes, buffer.size() - read_bytes, args)), ...);
  return read_bytes;
}

template<typename ...Args>
std::vector<std::byte> make_buffer(Args&&... args) {
  size_t const num_bytes = (serial_size(args) + ... );
  std::vector<std::byte> buffer(num_bytes);
  to_bytes(buffer, std::forward<Args>(args)...);
  return buffer;
}

template<typename ...Args>
constexpr size_t fill_buffer(std::vector<std::byte>& buffer, Args&&... args) {
  using namespace app_utils::serial;
  size_t const num_bytes = (serial_size(args) + ... );
  buffer.resize(num_bytes);
  return to_bytes(buffer, std::forward<Args>(args)...);
}

}  // namespace app_utils::serial
