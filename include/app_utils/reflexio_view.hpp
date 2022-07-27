#pragma once

#include <span>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include <app_utils/serial_utils.hpp>
#include <app_utils/cond_check.hpp>
#include "reflexio_iterator.hpp"

namespace reflexio {

template <typename ReflexioStruct>
struct reflexio_view {

  using Mask = typename ReflexioStruct::Mask;
  Mask exclude_mask;
  ReflexioStruct& object;

  constexpr reflexio_view(ReflexioStruct& object_,
                Mask exclude_mask_={})
  : exclude_mask(std::move(exclude_mask_))
  , object(object_) {}

  using Iterator = ReflexioIterator<std::decay_t<ReflexioStruct>>;
  constexpr Iterator begin() const { return Iterator(0, exclude_mask); }
  constexpr Iterator end  () const { return Iterator(ReflexioStruct::NumMemberVars); }

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

  constexpr friend size_t serial_size(reflexio_view const& view) {
    return 1 + app_utils::serial::serial_size(view.exclude_mask) +
           serial_size(view.object, view.exclude_mask);
  }

  constexpr friend size_t to_bytes(std::byte* buffer,
                         size_t const buffer_size,
                         reflexio_view const& instance) {

    size_t num_bytes = encode_mask({buffer, buffer_size}, instance.exclude_mask);
    num_bytes += to_bytes(buffer + num_bytes,
                          buffer_size - num_bytes,
                          instance.object,
                          instance.exclude_mask);
    return num_bytes;
  }

  constexpr friend size_t from_bytes(std::byte const* buffer,
                           size_t const buffer_size,
                           reflexio_view& val) {
    // Note: for backward compatibility, allow a serial size smaller than buffer size.
    size_t num_bytes = parse_mask({buffer, buffer_size}, val.exclude_mask);
    num_bytes += from_bytes(buffer + num_bytes,
                            buffer_size - num_bytes,
                            val.object,
                            val.exclude_mask);
    return num_bytes;
  }
};

// a version of reflexio_view that holds its own ReflexioStruct value
template <typename ReflexioStruct>
class reflexio_fat_view : public reflexio_view<ReflexioStruct> {
  // Note: that private member is referenced by object member
  // which is part of the public interface
  ReflexioStruct m_owned_object;

public:

  constexpr reflexio_fat_view(typename ReflexioStruct::Mask exclude_mask={})
      : reflexio_view<ReflexioStruct>(m_owned_object, std::move(exclude_mask))
      {}
};

} // namespace reflexio