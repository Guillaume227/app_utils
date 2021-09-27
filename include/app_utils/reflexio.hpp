#pragma once


#include <array>
#include <cstddef> // std::byte
#include <utility>


#ifndef REFLEXIO_MINIMAL_FEATURES
#ifdef __arm__
#define REFLEXIO_MINIMAL_FEATURES
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
  std::string_view const& get_name() const { return m_name; }
  std::string_view const& get_description() const { return m_description; }

  virtual std::string default_value_as_string() const = 0;
  virtual std::string value_as_string(void const* host) const = 0;
  virtual bool is_at_default(void const* host) const = 0;
  virtual bool values_differ(void const* host1, void const* host2) const = 0;
#endif

  virtual size_t get_serial_size(void const* host) const = 0;

  // returns number of bytes written
  virtual size_t write_to_bytes(std::byte* buffer, size_t buffer_size, void const* host) const = 0;
  // returns number of bytes read
  virtual size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, void* host) const = 0;

#ifdef DO_PYBIND_WRAPPING
  virtual void wrap_with_pybind(pybind11::module& pybindmodule_, void* pybindhost_) const = 0;
#endif
};

template <typename MemberType, typename HostType>
struct member_descriptor_impl_t : public member_descriptor_t {
  MemberType HostType::*const m_member_var_ptr;
#ifndef REFLEXIO_MINIMAL_FEATURES
  MemberType const m_default_value;
#endif

  template <typename... Args>
  constexpr member_descriptor_impl_t(MemberType HostType::*member_var_ptr,
#ifndef REFLEXIO_MINIMAL_FEATURES
                                     MemberType defaultValue,
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

  MemberType const& get_value(void const* host) const { 
    return static_cast<HostType const*>(host)->*m_member_var_ptr; 
  }

  MemberType& get_mutable_value(void* host) const { 
    return static_cast<HostType*>(host)->*m_member_var_ptr; 
  }

#ifndef REFLEXIO_MINIMAL_FEATURES
  std::string default_value_as_string() const override final {
    using namespace app_utils::strutils;
    return std::string{to_string(m_default_value)};
  }
  std::string value_as_string(void const* host) const override final {
    using namespace app_utils::strutils;
    return std::string{to_string(get_value(host))};
  }
  bool is_at_default(void const* host) const override { 
    return get_value(host) == m_default_value;
  }
  bool values_differ(void const* host1, void const* host2) const override final {
    return get_value(host1) != get_value(host2);
  }
#endif

  // returns number of bytes written
  size_t write_to_bytes(std::byte* buffer, size_t buffer_size, void const* host) const override {
    using namespace app_utils::serial;
    return to_bytes(buffer, buffer_size, get_value(host));
  }
  // return number of bytes read
  size_t read_from_bytes(std::byte const* buffer, size_t buffer_size, void* host) const override {
    using namespace app_utils::serial;
    return from_bytes(buffer, buffer_size, get_mutable_value(host));
  }

  size_t get_serial_size(void const* host) const override {
    using namespace app_utils::serial;
    return serial_size(get_value(host));
  }

#ifdef DO_PYBIND_WRAPPING
  void wrap_with_pybind(pybind11::module& pybindmodule_, void* pybindhost_) const override {
    auto* py_class = static_cast<HostType::PybindClassType*>(pybindhost_);
    using namespace app_utils::pybind_utils;
    pybind_wrapper<MemberType>::wrap_with_pybind(pybindmodule_);
    py_class->def_readwrite(get_name().data(), m_member_var_ptr, pybind_wrapper_traits<MemberType>::def_readwrite_rvp);
  }
#endif
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
  constexpr static size_t NumMemberVars = NumMemberVariables;
  static constexpr size_t num_registered_member_vars() { return NumMemberVariables; }

  static member_var_register_t const& get_member_descriptors() { 
    return CRTP::s_member_var_register;    
  }

  bool operator==(CRTP const& other) const {
    for (auto& descriptor : get_member_descriptors()) {
      if (descriptor->values_differ(this, &other)) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(CRTP const& other) const { return not(*this == other); }

#ifndef REFLEXIO_MINIMAL_FEATURES
  bool has_all_default_values() const {
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        return false;
      }
    }
    return true;
  }

  std::vector<std::string_view> non_default_values() const {
    std::vector<std::string_view> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
  }

  std::vector<std::string_view> differing_members(CRTP const& other) const {
    std::vector<std::string_view> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (descriptor->values_differ(this, &other)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
  }

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

  friend std::string to_string(CRTP const& instance) {
    std::ostringstream oss;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      oss << descriptor->get_name() << ": " << descriptor->value_as_string(&instance) << "\n";
    }
    return oss.str();
  }
#endif

  friend size_t serial_size(CRTP const& val) { return val.get_serial_size(); }

  size_t get_serial_size() const {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->get_serial_size(this);
    }
    return res;
  }

  // return number of written bytes
  friend size_t to_bytes(std::byte* buffer, size_t buffer_size, CRTP const& instance) {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->write_to_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "buffer is not big enough", buffer_size, '<', res);
    return res;
  }
  // return number of bytes read
  friend size_t from_bytes(std::byte const* buffer, size_t buffer_size, CRTP& instance) {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->read_from_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "buffer is not big enough", buffer_size, '<', res);
    return res;
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
  /* Making var_name ## _descr constexpr saves some space in an embedded context. */                \
  /* However it results in a test error on std::string member variable (default value is . */       \
  /* (default value remains empty string even when specified as something else). */                 \
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
                                                                                                    \
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

#ifdef DO_PYBIND_WRAPPING
namespace app_utils::pybind_utils {

namespace py = pybind11;

template <typename ReflexioStruct>
struct pybind_wrapper<ReflexioStruct,
                              std::enable_if_t<is_reflexio_struct<ReflexioStruct>::value, int>> {
  inline static bool s_registered_once = false;

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {

    if (not s_registered_once) {
      s_registered_once = true;
    
      static std::string const typeName = app_utils::typeName<ReflexioStruct>();
      auto wrappedType = typename ReflexioStruct::PybindClassType(pybindHost, typeName.c_str());
      wrappedType.def(pybind11::init<>())
          .def("__deepcopy__", [](ReflexioStruct const& self, py::dict) { return ReflexioStruct(self); })
          .def(pybind11::self == pybind11::self)
          .def(pybind11::self != pybind11::self)
          .def("__str__", [](ReflexioStruct const& self_) { return to_string(self_); })
          .def("get_serial_size", &ReflexioStruct::get_serial_size)
          .def("deserialize",
               [&](ReflexioStruct& self, pybind11::bytes const& data) {
                 std::string dataStr(data);
                 return from_bytes((std::byte const*)dataStr.c_str(), (unsigned int)dataStr.size(), self);
               })
          .def("serialize",
               [&](ReflexioStruct const& self) {
                 std::string dataStr(serial_size(self), '\0');
                 to_bytes((std::byte*)dataStr.c_str(), (unsigned int)dataStr.size(), self);
                 return pybind11::bytes(dataStr);
               })
          .def("non_default_values", &ReflexioStruct::non_default_values)
          .def("has_all_default_values", &ReflexioStruct::has_all_default_values)
          .def("differing_members", &ReflexioStruct::differing_members)
          .def("differences", &ReflexioStruct::differences);

      for (auto& member_descriptor : ReflexioStruct::get_member_descriptors()) {
        member_descriptor->wrap_with_pybind(pybindHost, &wrappedType);
      }
    }
  }
};
}  // namespace app_utils::pybind_utils
#endif
