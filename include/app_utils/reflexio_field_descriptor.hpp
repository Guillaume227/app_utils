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
#include <app_utils/yaml_utils.hpp>
#include <app_utils/cond_check.hpp>
#include "reflexio_traits.hpp"
#else
#define checkCond(...)
#endif
#include <app_utils/serial_utils.hpp>

namespace reflexio {

typedef void (*voidfunc)(void);

template<typename ReflexioStruct>
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

  template<typename T>
  T const& get_value_ref(ReflexioStruct const& host) const {
    return *reinterpret_cast<T const*>(get_value_void_ptr(host));
  }
  template<typename T>
  T& get_value_ref(ReflexioStruct& host) {
    return *reinterpret_cast<T*>(get_value_void_ptr(host));
  }

  constexpr virtual void const* get_value_void_ptr(ReflexioStruct const& host) const = 0;
  constexpr virtual void* get_value_void_ptr(ReflexioStruct& host) = 0;

  template<typename T>
  T const* get_min_value_ptr() const {
    if (auto* value_ptr = get_min_value_void_ptr()) {
      return reinterpret_cast<T const*>(value_ptr);
    }
    return nullptr;
  }
  template<typename T>
  T const* get_max_value_ptr() const {
    if (auto* value_ptr = get_max_value_void_ptr()) {
      return reinterpret_cast<T const*>(value_ptr);
    }
    return nullptr;
  }

  constexpr virtual void const* get_min_value_void_ptr() const = 0;
  constexpr virtual void const* get_max_value_void_ptr() const = 0;

  std::span<std::string_view const> get_values_str() const {
    if (auto* func_ptr = get_values_list_void_ptr()) {
      return (reinterpret_cast<std::span<std::string_view const> (*)()>(func_ptr))();
    }
    return {};
  }

  constexpr virtual voidfunc get_values_list_void_ptr() const = 0;

  [[nodiscard]]
  constexpr std::string_view const& get_name() const { return m_name; }
  [[nodiscard]]
  constexpr std::string_view const& get_description() const { return m_description; }
  // std::hash_code for the underlying type
  constexpr virtual size_t type_code() const = 0;
  constexpr virtual char const* type_name() const = 0;

  constexpr virtual std::string default_as_string() const = 0;
  constexpr virtual std::string value_as_string(ReflexioStruct const& host) const = 0;
  constexpr virtual void set_value_from_string(ReflexioStruct& host, std::string_view) const = 0;
  constexpr virtual void default_to_yaml(std::ostream&) const = 0;
  constexpr virtual void value_to_yaml(ReflexioStruct const& host, std::ostream&) const = 0;
  constexpr virtual void set_value_from_yaml(ReflexioStruct& host, std::istream&) const = 0;

  [[nodiscard]]
  constexpr virtual bool is_at_default(ReflexioStruct const& host) const = 0;
  constexpr virtual void set_to_default(ReflexioStruct& host) const = 0;
#endif
  [[nodiscard]]
  constexpr virtual size_t get_var_offset() const = 0;


#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  constexpr virtual bool values_differ(ReflexioStruct const& host1, ReflexioStruct const& host2) const = 0;
#endif
  [[nodiscard]]
  constexpr virtual size_t get_serial_size(ReflexioStruct const& host) const = 0;

  // returns number of bytes written
  constexpr virtual size_t write_to_bytes(std::byte* buffer, size_t buffer_size, ReflexioStruct const& host) const = 0;
  // returns number of bytes read
  constexpr virtual size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, ReflexioStruct& host) const = 0;

#ifdef DO_PYBIND_WRAPPING
  virtual void wrap_with_pybind(::pybind11::module& pybindmodule_, void* pybindhost_) const = 0;
  virtual ::pybind11::object get_py_value(ReflexioStruct const& host) const = 0;
  virtual void set_py_value(ReflexioStruct& host, pybind11::object const&) const = 0;
  virtual bool add_numpy_descriptor(std::vector<::pybind11::detail::field_descriptor>&) const = 0;
#endif
};

template <typename ReflexioStruct, typename MemberType>
struct member_descriptor_impl_t : public member_descriptor_t<ReflexioStruct> {
  MemberType ReflexioStruct::*const m_member_var_ptr;
  using MemberTypeValFunc = MemberType const& (*)();
  using MemberTypeSpanFunc = std::span<MemberType const> const& (*)();

#ifndef REFLEXIO_MINIMAL_FEATURES
  typename reflexio_traits<MemberType>::DefaultType const m_default_value;
  MemberTypeValFunc const m_min_value_func = nullptr;
  MemberTypeValFunc const m_max_value_func = nullptr;
  MemberTypeSpanFunc const m_values_list_func = nullptr;
#endif

  constexpr member_descriptor_impl_t(
    MemberType ReflexioStruct::*member_var_ptr,
    std::string_view name,
    std::string_view description,
#ifndef REFLEXIO_MINIMAL_FEATURES
    typename reflexio_traits<MemberType>::DefaultType defaultValue,
    MemberTypeValFunc min_value_func = nullptr,
    MemberTypeValFunc max_value_func = nullptr
#else
    MemberType /*defaultValue*/,
    MemberTypeValFunc /*min_value_func*/ = nullptr,
    MemberTypeValFunc /*max_value_func*/ = nullptr
#endif
    )
      : member_descriptor_t<ReflexioStruct>(name, description)
      , m_member_var_ptr(member_var_ptr)
#ifndef REFLEXIO_MINIMAL_FEATURES
      , m_default_value(std::move(defaultValue))
      , m_min_value_func(min_value_func)
      , m_max_value_func(max_value_func)
#endif
  {}

  // explicit definition required because of gcc bug: https://gcc.gnu.org/bugzilla/show_bug.cgi?id=93413
  constexpr ~member_descriptor_impl_t() override = default;

  [[nodiscard]]
  constexpr MemberType const& get_value(ReflexioStruct const& host) const {
    return host.*m_member_var_ptr;
  }

  [[nodiscard]]
  constexpr MemberType& get_mutable_value(ReflexioStruct& host) const {
    return host.*m_member_var_ptr;
  }

  [[nodiscard]]
  constexpr size_t get_var_offset() const final {
    return ReflexioStruct::offset_of(m_member_var_ptr);
  }
#ifndef REFLEXIO_MINIMAL_FEATURES
  constexpr void* get_value_void_ptr(ReflexioStruct& host) final {
    return &(host.*m_member_var_ptr);
  }
  constexpr void const* get_value_void_ptr(ReflexioStruct const& host) const final {
    return &(host.*m_member_var_ptr);
  }

  constexpr size_t type_code() const final {
    return typeid(MemberType).hash_code();
  }

  constexpr char const* type_name() const final {
    return app_utils::typeName<MemberType>().data();
  }

  constexpr void const* get_min_value_void_ptr() const final {
    return m_min_value_func ? &m_min_value_func() : nullptr;
  }

  constexpr void const* get_max_value_void_ptr() const final {
    return m_max_value_func ? &m_max_value_func() : nullptr;
  }

  constexpr voidfunc get_values_list_void_ptr() const final {
    return (voidfunc)reflexio::reflexio_traits<MemberType>::get_values;
  }

  [[nodiscard]]
  constexpr bool is_at_default(ReflexioStruct const& host) const final {
    return get_value(host) == m_default_value;
  }
  constexpr void set_to_default(ReflexioStruct& host) const final {
    get_mutable_value(host) = m_default_value;
  }

  constexpr std::string default_as_string() const final {
    using namespace app_utils::strutils;
    return std::string{to_string(m_default_value)};
  }
  constexpr std::string value_as_string(ReflexioStruct const& host) const final {
    using namespace app_utils::strutils;
    return std::string{to_string(get_value(host))};
  }
  constexpr void set_value_from_string(ReflexioStruct& host, std::string_view val_str) const final {
    using namespace app_utils::strutils;
    from_string(get_mutable_value(host), val_str);
  }

  constexpr void default_to_yaml(std::ostream& os) const final {
    using namespace yaml_utils;
    to_yaml(m_default_value, os);
  }
  constexpr void value_to_yaml(ReflexioStruct const& host, std::ostream& os) const final {
    using namespace yaml_utils;
    to_yaml(get_value(host), os);
  }
  constexpr void set_value_from_yaml(ReflexioStruct& host, std::istream& stream) const final {
    using namespace yaml_utils;
    from_yaml(get_mutable_value(host), stream);
  }

#endif
#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  constexpr bool values_differ(ReflexioStruct const& host1, ReflexioStruct const& host2) const final {
    return get_value(host1) != get_value(host2);
  }
#endif
  // returns number of bytes written
  constexpr size_t write_to_bytes(std::byte* buffer, size_t buffer_size, ReflexioStruct const& host) const final {
    using namespace app_utils::serial;
    return to_bytes(buffer, buffer_size, get_value(host));
  }
  // return number of bytes read
  constexpr size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, ReflexioStruct& host) const final {
    using namespace app_utils::serial;
    return from_bytes(buffer, buffer_size, get_mutable_value(host));
  }

  [[nodiscard]]
  constexpr size_t get_serial_size(ReflexioStruct const& host) const final {
    using namespace app_utils::serial;
    if constexpr (std::is_standard_layout<MemberType>()) {
      return serial_size(MemberType{});
    } else {
      return serial_size(get_value(host));
    }
  }

#ifdef DO_PYBIND_WRAPPING
  void wrap_with_pybind(pybind11::module& pybindmodule_, void* pybindhost_) const final {
    auto* py_class = static_cast<ReflexioStruct::PybindClassType*>(pybindhost_);
    using namespace app_utils::pybind;
    pybind_wrapper<MemberType>::wrap_with_pybind(pybindmodule_);
    py_class->def_readwrite(this->get_name().data(), m_member_var_ptr, pybind_wrapper_traits<MemberType>::def_readwrite_rvp);
  }

  pybind11::object get_py_value(ReflexioStruct const& host) const final {
    return pybind11::cast(&get_value(host));
  }

  void set_py_value(ReflexioStruct& host, pybind11::object const& obj) const final {
      get_mutable_value(host) = obj.cast<MemberType>();
  }

  bool add_numpy_descriptor(std::vector<::pybind11::detail::field_descriptor>& vect) const final {
    if constexpr(std::is_standard_layout<MemberType>()) {
      vect.emplace_back(this->get_name().data(),
                        ((::pybind11::ssize_t) &reinterpret_cast<char const volatile&>((((ReflexioStruct*) 0)->*
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
}  // namespace reflexio
