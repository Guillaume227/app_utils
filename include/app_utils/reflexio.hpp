#pragma once


#include <array>
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

//namespace app_utils::reflexio {
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


template <typename ReflexioStruct>
class ReflexioIterator {
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::string_view const;
  using pointer = value_type*;    // or also value_type*
  using reference = value_type&;  // or also value_type&

  size_t m_idx;

 public:
  ReflexioIterator(size_t idx = 0) : m_idx(idx) {}

  reference operator*() const { return ReflexioStruct::get_member_descriptors()[m_idx]->get_name(); }
  pointer operator->() { return &ReflexioStruct::get_member_descriptors()[m_idx]->get_name(); }

  ReflexioIterator& operator++() {
    m_idx++;
    return *this;
  }

  ReflexioIterator operator++(int) {
    ReflexioIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  friend bool operator==(ReflexioIterator const& a, ReflexioIterator const& b) { return a.m_idx == b.m_idx; };
  friend bool operator!=(ReflexioIterator const& a, ReflexioIterator const& b) { return a.m_idx != b.m_idx; };
};

template <typename CRTP, size_t NumMemberVariables>
struct ReflexioStructBase {
  using ReflexioTypeName = CRTP;
#ifdef DO_PYBIND_WRAPPING
  using PybindClassType = pybind11::class_<CRTP>;
#endif

 protected:
  using member_var_register_t = std::array<member_descriptor_t const*, NumMemberVariables>;

 public:
  static constexpr size_t NumMemberVars = NumMemberVariables;
  static constexpr size_t num_registered_member_vars() { return NumMemberVariables; }

  constexpr static member_var_register_t const& get_member_descriptors() {
    return CRTP::s_member_var_register;    
  }

#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  friend constexpr bool operator==(CRTP const& self, CRTP const& other) {
    for (auto& descriptor : self.get_member_descriptors()) {
      if (descriptor->values_differ(&self, &other)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]]
  friend constexpr bool operator!=(CRTP const& self, CRTP const& other) {
    return not(self == other); }
#endif

#ifndef REFLEXIO_MINIMAL_FEATURES

  using Iterator = ReflexioIterator<CRTP>;
  Iterator begin() const { return Iterator(0); }
  Iterator end  () const { return Iterator(NumMemberVars); }

  [[nodiscard]]
  constexpr bool has_all_default_values() const {
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]]
  std::vector<std::string_view> non_default_values() const {
    std::vector<std::string_view> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::vector<std::string_view> differing_members(CRTP const& other) const {
    std::vector<std::string_view> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (descriptor->values_differ(this, &other)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::string differences(CRTP const& other) const {
    std::ostringstream out;
    for (auto& descriptor : get_member_descriptors()) {
      if (descriptor->values_differ(this, &other)) {
        out << descriptor->get_name() << ": " 
            << descriptor->value_as_string(this) << " vs "
            << descriptor->value_as_string(&other) << '\n';
      }
    }
    return out.str();
  }

  [[nodiscard]]
  constexpr friend std::string to_string(CRTP const& instance) {
    std::ostringstream oss;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      oss << descriptor->get_name() << ": " << descriptor->value_as_string(&instance) << "\n";
    }
    return oss.str();
  }

  static std::string const& get_docstring() {
    static std::string const docstring = [] {
      std::ostringstream oss;
      for (auto& descriptor: CRTP::get_member_descriptors()) {
        oss << descriptor->get_name() << ": " << descriptor->get_description() << "\n";
      }
      return oss.str();
    }();
    return docstring;
  }

#endif

  [[nodiscard]]
  friend constexpr size_t serial_size(CRTP const& val) { return val.get_serial_size(); }

  [[nodiscard]]
  constexpr size_t get_serial_size() const {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->get_serial_size(this);
    }
    return res;
  }

  // return number of written bytes
  friend size_t to_bytes(std::byte* buffer, size_t const buffer_size, CRTP const& instance) {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->write_to_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "output buffer is not big enough to fit object", buffer_size, '<', res);
    return res;
  }
  friend size_t to_bytes(std::span<std::byte> buffer, CRTP& instance) {
    return to_bytes(buffer.data(), buffer.size(), instance);
  }

  // return number of bytes read
  friend size_t from_bytes(std::byte const* buffer, size_t const buffer_size, CRTP& instance) {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->read_from_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "input buffer has less data than required:", buffer_size, '<', res, 
      ". Look for inconsistent serialization/deserialization of", app_utils::typeName<CRTP>());
    return res; //TODO: revisit, saw mismatch between buffer size (383) and read byte (386) buffer_size >= res ? res : 0;
  }
  friend size_t from_bytes(std::span<std::byte const> buffer, CRTP& instance) {
    return from_bytes(buffer.data(), buffer.size(), instance);
  }
};

constexpr size_t count_member_var_declarations(std::string_view const text) {
  size_t count = 0;

  std::string_view const register_member_str = "REFLEXIO_MEMBER_VAR_DEFINE";

  for (size_t i = 0; i < text.size(); i++) {
    if (text.substr(i, register_member_str.size()) == register_member_str) {
      count++;
    }
  }
  return count;
}

template <typename T>
using is_reflexio_struct = std::is_base_of<ReflexioStructBase<T, T::NumMemberVars>, T>;

//}  // namespace app_utils::reflexio

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description)                  \
  var_type var_name = var_type(default_value);                                                      \
                                                                                                    \
  inline static constexpr auto __##var_name##_descr = [] {                                          \
    return member_descriptor_impl_t<var_type, ReflexioTypeName>{&ReflexioTypeName::var_name,        \
                                                                default_value,                      \
                                                                #var_name,                          \
                                                                description};                       \
  }();                                                                                              \
                                                                                                    \
  static constexpr int __##var_name##_id = __COUNTER__;                                             \
                                                                                                    \
  template <class Dummy>                                                                            \
  struct member_var_counter_t<__##var_name##_id, Dummy> {                                           \
    static constexpr int index = member_var_counter_t<__##var_name##_id - 1, Dummy>::index + 1;     \
  };                                                                                                \
  template <class Dummy>                                                                            \
  struct member_var_descriptor_t<member_var_counter_t<__##var_name##_id, int>::index, Dummy> {      \
    static constexpr member_descriptor_t const* descriptor = &__##var_name##_descr;                 \
  }

// define a member variable with a 'default default'
#define REFLEXIO_MEMBER_VAR_DEFINE_DEF(var_type, var_name, description) \
  REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, var_type(), description)

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)                                                     \
  struct StructName : ReflexioStructBase<StructName, count_member_var_declarations(#__VA_ARGS__)> { \
    template <size_t N, class dummy>                                                                \
    struct member_var_descriptor_t {                                                                \
    };                                                                                              \
    template <int N, class Dummy=int>                                                               \
    struct member_var_counter_t {                                                                   \
      static constexpr int index = member_var_counter_t<N - 1, Dummy>::index;                       \
    };                                                                                              \
    template <class Dummy>                                                                          \
    struct member_var_counter_t<-1, Dummy> {                                                        \
      static constexpr int index = -1;                                                              \
    };                                                                                              \
    constexpr StructName() = default;                                                               \
    __VA_ARGS__                                                                                     \
                                                                                                    \
    inline static constexpr member_var_register_t s_member_var_register =                           \
        []<size_t... NN>(std::index_sequence<NN...>){                                               \
      member_var_register_t out{nullptr};                                                           \
      std::size_t i = 0;                                                                            \
      (void(out[i++] = member_var_descriptor_t<NN, int>::descriptor), ...);                         \
      return out;                                                                                   \
    }(std::make_index_sequence<num_registered_member_vars()>());                                    \
  }
