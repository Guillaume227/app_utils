#pragma once

#include <app_utils/cond_check.hpp>
#include <app_utils/string_utils.hpp>
#include <string>
#include <memory>
#include <array>
#include <typeinfo>
#include <sstream>

struct member_descriptor_t {

  std::string const m_name;
  std::string const m_description;

  constexpr member_descriptor_t(std::string name,                                
                                std::string description)
      
    : m_name(std::move(name))
    , m_description(std::move(description))
  {}

  virtual ~member_descriptor_t() = default;

  virtual std::type_info const& get_member_type_info() const = 0;
  virtual std::string default_value_as_string() const = 0;
  virtual std::string value_as_string(void const* host) const = 0;
  virtual bool is_at_default(void const* host) const = 0;
  virtual bool values_differ(void const* host1, void const* host2) const = 0;

  std::string const& get_name() const { return m_name; }
  std::string const& get_description() const { return m_description; }


  /*
  virtual std::bytes value_as_bytes() const = 0;
  virtual set_value(std::bytes);
  virtual std::string value_as_string() const = 0;
  virtual set_value(std::string);
  */
  
 #ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
  virtual void wrap_in_pybind(void* pybindclass_) const = 0;
#endif
};


template<typename MemberType, typename HostType>
struct member_descriptor_impl_t : public member_descriptor_t {

  MemberType HostType::* const m_member_var_ptr;
  MemberType const m_default_value; 

  template<typename ...Args>
  constexpr member_descriptor_impl_t(MemberType HostType::*member_var_ptr, MemberType defaultValue, Args&& ...args)
      : member_descriptor_t(std::forward<Args>(args)...)
      , m_default_value(std::move(defaultValue))
      , m_member_var_ptr(member_var_ptr)
  {}

  std::type_info const& get_member_type_info() const override { return typeid(MemberType); }
  std::string default_value_as_string() const override { 
    using namespace app_utils::strutils;
    return to_string(m_default_value); 
  }
  std::string value_as_string(void const* host) const override {
    using namespace app_utils::strutils;
    return to_string(get_value(host));
  }

  MemberType const& get_value(void const* host) const { 
    return static_cast<HostType const*>(host)->*m_member_var_ptr;
  }

  bool values_differ(void const* host1, void const* host2) const override {
    return get_value(host1) != get_value(host2);
  }
  bool is_at_default(void const* host) const override { 
    return get_value(host) == m_default_value;
  }

#ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
  void wrap_in_pybind(void* pybindclass_) const override {
    static_cast<HostType::PybindClassType*>(pybindclass_)->def_readwrite(get_name().c_str(), m_member_var_ptr);
  }
#endif
};

/*
#define REFLEXIO_STRUCT_DEFINE(StructName,
REFLEXIO_MEMBER_VAR_DEFINE(type1, member1, default_val1, description1);
REFLEXIO_MEMBER_VAR_DEFINE(type2, member2, default_val2, description2);
);
*/

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description) \
  struct var_name ## _descriptor_t {\
    static void register_descriptor(ReflexioStructBase& reflexioStruct) { \
      static bool registered = (TypeName::register_member(                                                               \
              std::make_unique<member_descriptor_impl_t<var_type, TypeName>>(&TypeName::var_name, default_value, #var_name, description)), \
                                true); \
    } \
  };\
  var_type var_name = (var_name##_descriptor_t :: register_descriptor(*this), default_value)


template<typename CRTP, size_t NumMemberVariables>
struct ReflexioStructBase {
  using TypeName = CRTP;
#ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
  using PybindClassType = pybind11::class_<CRTP>;
#endif

 private:
  using member_register_t = std::array<std::unique_ptr<member_descriptor_t const>, NumMemberVariables>;  
  inline static member_register_t s_member_register;

 protected:
  static void register_member(std::unique_ptr<member_descriptor_t> member_descriptor) { 
    for (auto& slot : s_member_register) {
      if (slot == nullptr) {
        slot = std::move(member_descriptor);
        break;
      }
    }
  }

 public:
  static member_register_t const& get_member_descriptors() { 
    static CRTP type; // forces population of member register once
    return s_member_register; 
  }

  bool has_all_default_values() const {
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        return false;
      }
    }
    return true;
  }

  std::vector<std::string> non_default_values() const {
    std::vector<std::string> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (not descriptor->is_at_default(this)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
  }

  std::vector<std::string> differing_values(CRTP const& other) const {
    std::vector<std::string> res;
    for (auto& descriptor : get_member_descriptors()) {
      if (descriptor->values_differ(this, &other)) {
        res.push_back(descriptor->get_name());
      }
    }
    return res;
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

  friend std::string to_string(CRTP const& instance) { 
    std::ostringstream oss;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      oss << descriptor->get_name() << ": " << descriptor->value_as_string(&instance) << "\n";
    }
    return oss.str();
  }

  static constexpr size_t num_members() { return NumMemberVariables; }
};

constexpr size_t count_chars(std::string const text, char char_to_count) {
  size_t count = 0;

  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == char_to_count) {
      count++;
    }
  }
  return count;
}

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)  \
  struct StructName : ReflexioStructBase<StructName, count_chars(#__VA_ARGS__, ';')> {\
    __VA_ARGS__;                                 \
  }

#ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
namespace pybind11 {

template <typename ReflexioStruct, typename PyModule>
void wrap_reflexio_struct(PyModule& pymodule) {
  static std::string const typeName = app_utils::typeName<ReflexioStruct>();
  auto wrappedType = typename ReflexioStruct::PybindClassType(pymodule, typeName.c_str());
  wrappedType
    .def(pybind11::init<>())
    .def(pybind11::self == pybind11::self)
    .def(pybind11::self != pybind11::self)
    .def("__str__", [](ReflexioStruct const& self) { return to_string(self);  })
    .def("non_default_values", &ReflexioStruct::non_default_values)
    .def("has_all_default_values", &ReflexioStruct::has_all_default_values)
    .def("differing_values", &ReflexioStruct::differing_values);

  for (auto& member_descriptor : ReflexioStruct::get_member_descriptors()) {
    member_descriptor->wrap_in_pybind(&wrappedType);
  }
}
}  // namespace pybind11
#endif
