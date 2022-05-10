#pragma once


#include <array>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>


#ifndef REFLEXIO_MINIMAL_FEATURES
#ifdef __arm__
#define REFLEXIO_MINIMAL_FEATURES
#ifndef REFLEXIO_WITH_COMPARISON_OPERATORS
#define REFLEXIO_NO_COMPARISON_OPERATORS
#endif
#endif
#endif


#ifndef REFLEXIO_MINIMAL_FEATURES
#include <string_view>
#include <typeinfo>
#include <sstream>
#include <app_utils/string_utils.hpp>
#include <app_utils/cond_check.hpp>
#else
#define checkCond(...)
#endif
#include <app_utils/serial_utils.hpp>

namespace app_utils::reflexio {
struct member_descriptor_t {
#ifndef REFLEXIO_MINIMAL_FEATURES
  std::string_view const m_name;
  std::string_view const m_description;

  constexpr member_descriptor_t(std::string_view name,
                                std::string_view description)
    : m_name(name)
    , m_description(description)
  {}
#else
  constexpr member_descriptor_t(std::string_view, std::string_view) {}
#endif

  constexpr virtual ~member_descriptor_t() = default;

#ifndef REFLEXIO_MINIMAL_FEATURES
  [[nodiscard]]
  constexpr std::string_view const& get_name() const { return m_name; }
  [[nodiscard]]
  constexpr std::string_view const& get_description() const { return m_description; }

  [[nodiscard]]
  constexpr virtual std::string default_value_as_string() const = 0;
  [[nodiscard]]
  constexpr virtual std::string value_as_string(void const* host) const = 0;
  [[nodiscard]]
  constexpr virtual bool is_at_default(void const* host) const = 0;
  [[nodiscard]]
  constexpr virtual size_t get_var_offset() const = 0;
#endif


#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  constexpr virtual bool values_differ(void const* host1, void const* host2) const = 0;
#endif
  [[nodiscard]]
  constexpr virtual size_t get_serial_size(void const* host) const = 0;

  // returns number of bytes written
  constexpr virtual size_t write_to_bytes(std::byte* buffer, size_t buffer_size, void const* host) const = 0;
  // returns number of bytes read
  constexpr virtual size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, void* host) const = 0;

#ifdef DO_PYBIND_WRAPPING
  virtual void wrap_with_pybind(::pybind11::module& pybindmodule_, void* pybindhost_) const = 0;
  virtual ::pybind11::object get_py_value(void const* host) const = 0;
  virtual void set_py_value(void* host, pybind11::object const&) const = 0;
  virtual bool add_numpy_descriptor(std::vector<::pybind11::detail::field_descriptor>&) const = 0;
#endif
};

#ifndef REFLEXIO_MINIMAL_FEATURES
/*
  DefaultType: work around constexpr transient allocation for std::string.
  See reply there: https://stackoverflow.com/a/69590837/4249338
  Something similar should be done for e.g. std::vector.
*/
template<typename T>
struct reflexio_traits {
  using DefaultType = T;
};

template <>
struct reflexio_traits<std::string> {
  using DefaultType = std::string_view;
};

#endif

template <typename MemberType, typename HostType>
struct member_descriptor_impl_t : public member_descriptor_t {
  MemberType HostType::*const m_member_var_ptr;
#ifndef REFLEXIO_MINIMAL_FEATURES
  typename reflexio_traits<MemberType>::DefaultType const m_default_value;
#endif

  template <typename... Args>
  constexpr member_descriptor_impl_t(
    MemberType HostType::*member_var_ptr,
#ifndef REFLEXIO_MINIMAL_FEATURES
      typename reflexio_traits<MemberType>::DefaultType defaultValue,
#else
      MemberType /*defaultValue*/,
#endif
    Args&& ...args)
      : member_descriptor_t(std::forward<Args>(args)...)
      , m_member_var_ptr(member_var_ptr)
#ifndef REFLEXIO_MINIMAL_FEATURES
      , m_default_value(std::move(defaultValue))
#endif
  {}

  // explicit definition required because of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
  constexpr ~member_descriptor_impl_t() override = default;

  [[nodiscard]]
  constexpr MemberType const& get_value(void const* host) const {
    return static_cast<HostType const*>(host)->*m_member_var_ptr;
  }

  [[nodiscard]]
  constexpr MemberType& get_mutable_value(void* host) const {
    return static_cast<HostType*>(host)->*m_member_var_ptr; 
  }

#ifndef REFLEXIO_MINIMAL_FEATURES

  [[nodiscard]]
  constexpr size_t get_var_offset() const final {
    return HostType::offset_of(m_member_var_ptr);
  }

  [[nodiscard]]
  constexpr std::string default_value_as_string() const final {
    using namespace app_utils::strutils;
    return std::string{to_string(m_default_value)};
  }
  [[nodiscard]]
  constexpr std::string value_as_string(void const* host) const final {
    using namespace app_utils::strutils;
    return std::string{to_string(get_value(host))};
  }
  [[nodiscard]]
  constexpr bool is_at_default(void const* host) const final {
    return get_value(host) == m_default_value;
  }
#endif
#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  constexpr bool values_differ(void const* host1, void const* host2) const final {
    return get_value(host1) != get_value(host2);
  }
#endif
  // returns number of bytes written
  constexpr size_t write_to_bytes(std::byte* buffer, size_t buffer_size, void const* host) const final {
    using namespace app_utils::serial;
    return to_bytes(buffer, buffer_size, get_value(host));
  }
  // return number of bytes read
  constexpr size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, void* host) const final {
    using namespace app_utils::serial;
    return from_bytes(buffer, buffer_size, get_mutable_value(host));
  }

  [[nodiscard]]
  constexpr size_t get_serial_size(void const* host) const final {
    using namespace app_utils::serial;
    if constexpr (std::is_standard_layout<MemberType>()) {
      return serial_size(MemberType{});
    } else {
      return serial_size(get_value(host));
    }
  }

#ifdef DO_PYBIND_WRAPPING
  void wrap_with_pybind(pybind11::module& pybindmodule_, void* pybindhost_) const final {
    auto* py_class = static_cast<HostType::PybindClassType*>(pybindhost_);
    using namespace app_utils::pybind;
    pybind_wrapper<MemberType>::wrap_with_pybind(pybindmodule_);
    py_class->def_readwrite(get_name().data(), m_member_var_ptr, pybind_wrapper_traits<MemberType>::def_readwrite_rvp);
  }

  pybind11::object get_py_value(void const* host) const final { 
    return pybind11::cast(&get_value(host));
  }

  void set_py_value(void* host, pybind11::object const& obj) const final {
      get_mutable_value(host) = obj.cast<MemberType>();
  }

  bool add_numpy_descriptor(std::vector<::pybind11::detail::field_descriptor>& vect) const final {
    if constexpr(std::is_standard_layout<MemberType>()) {
      vect.emplace_back(m_name.data(),
                        ((::pybind11::ssize_t) &reinterpret_cast<char const volatile&>((((HostType*) 0)->*
                                                                                        m_member_var_ptr))),
                        sizeof(MemberType),
                        ::pybind11::format_descriptor<MemberType>::format(),
                        ::pybind11::detail::npy_format_descriptor<MemberType>::dtype());
      return true;
    } else {
      return false;
    }
  }
#endif
};
}  // namespace app_utils::reflexio
