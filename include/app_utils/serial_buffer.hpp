#pragma once

#include <cstddef> // std::byte
#include <span>

// to be included after serial_utils.hpp

namespace app_utils::serial {

template<typename Arg1, typename Arg2, typename ...Args>
constexpr size_t serial_size(Arg1 const& arg1, Arg2 const& arg2, Args const& ... args) {
  return serial_size(arg1) + serial_size(arg2) + (serial_size(args) + ... + 0 );
}

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
  size_t const num_bytes = serial_size(std::forward<Args>(args)...);
  std::vector<std::byte> buffer(num_bytes);
  to_bytes(buffer, std::forward<Args>(args)...);
  return buffer;
}

template<typename ...Args>
void append_to_buffer(std::vector<std::byte>& buffer, Args&&... args) {
  size_t const num_bytes = serial_size(std::forward<Args>(args)...);
  size_t initial_size = buffer.size();
  buffer.resize(initial_size + num_bytes);
  to_bytes({buffer.data() + initial_size, num_bytes}, std::forward<Args>(args)...);
}

template<typename ...Args>
constexpr size_t fill_buffer(std::vector<std::byte>& buffer, Args&&... args) {
  using namespace app_utils::serial;
  size_t const num_bytes = serial_size(std::forward<Args>(args)...);
  buffer.resize(num_bytes);
  return to_bytes(buffer, std::forward<Args>(args)...);
}

}  // namespace app_utils::serial
