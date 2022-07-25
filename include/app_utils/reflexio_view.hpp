#pragma once

#include <span>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include <app_utils/serial_utils.hpp>
#include <app_utils/cond_check.hpp>

namespace reflexio {

template <typename ReflexioStruct>
struct reflexio_view {

  using Mask = typename ReflexioStruct::Mask;
  Mask exclude_mask;
  ReflexioStruct& reflexio_struct;

  reflexio_view(ReflexioStruct& reflexio_struct_,
                Mask exclude_mask_={})
  : exclude_mask(std::move(exclude_mask_))
  , reflexio_struct(reflexio_struct_) {}

  static size_t parse_mask(std::span<std::byte const> buffer,
                           Mask& exclude_mask) {

    size_t const serial_mask_size = (size_t) buffer[0];
    if (serial_mask_size == 0) {
      exclude_mask.reset();
      return 1;
    }
    checkCond(serial_mask_size <= buffer.size() - 1,
              "mask size doesn't fit in buffer:",
              serial_mask_size, ">", buffer.size() - 1);
    size_t num_bytes = app_utils::serial::from_bytes(buffer.data() + 1,
                                                     serial_mask_size,
                                                     exclude_mask);
    return num_bytes + 1;
  }

  static size_t encode_mask(std::span<std::byte> buffer,
                            Mask const& exclude_mask) {
    if (exclude_mask.none()) {
      buffer[0] = std::byte{0};
      return 1;
    }

    size_t mask_size = app_utils::serial::serial_size(exclude_mask);
    checkCond(mask_size < 0xFF);
    buffer[0] = (std::byte) mask_size;
    return 1 + app_utils::serial::to_bytes(buffer.data() + 1,
                                           buffer.size() - 1,
                                           exclude_mask);
  }

  friend size_t serial_size(reflexio_view const& view) {
    return 1 + app_utils::serial::serial_size(view.exclude_mask) +
           serial_size(view.reflexio_struct, view.exclude_mask);
  }

  friend size_t to_bytes(std::byte* buffer,
                         size_t const buffer_size,
                         reflexio_view const& instance) {

    size_t num_bytes = encode_mask({buffer, buffer_size}, instance.exclude_mask);
    num_bytes += to_bytes(buffer + num_bytes,
                          buffer_size - num_bytes,
                          instance.reflexio_struct,
                          instance.exclude_mask);
    return num_bytes;
  }

  friend size_t from_bytes(std::byte const* buffer,
                           size_t const buffer_size,
                           reflexio_view& val) {
    // note: for backward compatibility, allow a serial size smaller than buffer size.
    size_t num_bytes = parse_mask({buffer, buffer_size}, val.exclude_mask);
    num_bytes += from_bytes(buffer + num_bytes,
                            buffer_size - num_bytes,
                            val.reflexio_struct,
                            val.exclude_mask);
    return num_bytes;
  }
};

} // namespace reflexio
