#pragma once

#include <span>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include <app_utils/serial_utils.hpp>
#include <app_utils/cond_check.hpp>
#include "reflexio_iterator.hpp"

namespace reflexio {

template <typename ReflexioStruct, typename ...T>
void set(typename ReflexioStruct::Mask& mask, T ReflexioStruct::* ...varPtr) {
  (mask.set(ReflexioStruct::index_of_var(varPtr), true), ...);
}

template <typename ReflexioStruct, typename ...T>
void test(typename ReflexioStruct::Mask& mask, T ReflexioStruct::* ...varPtrs) {
  return (mask.test(ReflexioStruct::index_of_var(varPtrs)) || ...);
}

template <typename ReflexioStruct>
struct reflexio_view {

  using ReflexioT = ReflexioStruct;
  using Mask = typename ReflexioStruct::Mask;
  Mask exclude_mask;
  ReflexioStruct& object;

  constexpr reflexio_view(ReflexioStruct& object_,
                          Mask exclude_mask_={})
  : exclude_mask(std::move(exclude_mask_))
  , object(object_) {}

  //constexpr reflexio_view(reflexio_view const&) = default;
  constexpr reflexio_view(reflexio_view<std::remove_const_t<ReflexioStruct>> const& other)
  : reflexio_view(other.object, other.exclude_mask) {}

  using Iterator = ReflexioIterator<std::remove_const_t<ReflexioStruct>>;
  constexpr Iterator begin() const { return Iterator(0, exclude_mask); }
  constexpr Iterator end  () const { return Iterator(ReflexioStruct::NumMemberVars); }

  // number of fields present in the view
  [[nodiscard]]
  size_t size() const {
    return exclude_mask.size() - exclude_mask.count();
  }

  template<typename VarPtr>
  [[nodiscard]]
  bool has(VarPtr const& varPtr) const {
    return not exclude_mask.test(ReflexioStruct::index_of_var(varPtr));
  }

  // copy only the masked fields
  reflexio_view& operator=(ReflexioStruct const& s) {
    for (auto& descriptor: *this) {
      descriptor.copy_value(&object, &s);
    }
    return *this;
  }

  template<typename T>
  void set(T ReflexioStruct::* varPtr, bool value) {
    exclude_mask.set(ReflexioStruct::index_of_var(varPtr), not value);
  }

  template<typename ...T>
  void include(T ReflexioStruct::* ... varPtr) {
    (set(varPtr, false), ...);
  }

  template<typename ...T>
  void include_only(T ReflexioStruct::* ... varPtr) {
    exclude_mask.set(true);
    include(varPtr...);
  }

  template<typename T>
  void exclude(T ReflexioStruct::* varPtr) {
    set(varPtr, true);
  }

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
    return (view.exclude_mask.none() ? 2 : (1 + app_utils::serial::serial_size(view.exclude_mask))) +
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
  // Note: that private member is referenced by parent member reflexio_view::object
  // which is part of the public interface
  ReflexioStruct m_owned_object;

public:

  constexpr reflexio_fat_view(typename ReflexioStruct::Mask const& mask=ReflexioStruct::exclude_none)
    : reflexio_view<ReflexioStruct>(m_owned_object, mask)
  {}

  constexpr reflexio_fat_view(reflexio_fat_view const& obj)
    : reflexio_view<ReflexioStruct>(m_owned_object, obj.exclude_mask)
    , m_owned_object(obj.object)
  {}

  constexpr reflexio_fat_view& operator=(reflexio_fat_view const& obj) {
    m_owned_object = obj.m_owned_object;
    this->exclude_mask = obj.exclude_mask;
    return *this;
  }
};

} // namespace reflexio
