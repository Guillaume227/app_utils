#pragma once

#include <app_utils/cond_check.hpp>
#include <app_utils/string_utils.hpp>
#include <array>
#include <vector>
#include <string>
#include <typeinfo>
#include <memory>

struct member_descriptor_t {

  std::string const m_name;
  std::string const m_default_str;
  std::string const m_description;

  constexpr member_descriptor_t(std::string name, 
                                std::string default_str, 
                                std::string description)
      
    : m_name(std::move(name))
    , m_default_str(std::move(default_str))
    , m_description(std::move(description))
  {}

  virtual ~member_descriptor_t() = default;

  virtual std::type_info const& get_member_type_info() const = 0;

  std::string const& get_name() const { return m_name; }
  std::string const& get_description() const { return m_description; }
  std::string const& default_as_string() const { return m_default_str; }

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

  MemberType HostType::* m_member_var_ptr;

  template<typename ...Args>
  constexpr member_descriptor_impl_t(MemberType HostType::*member_var_ptr, Args&& ...args)
      : member_descriptor_t(std::forward<Args>(args)...)
      , m_member_var_ptr(member_var_ptr)
  {}
  std::type_info const& get_member_type_info() const override { return typeid(MemberType); }

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
              std::make_unique<member_descriptor_impl_t<var_type, TypeName>>(&TypeName::var_name, #var_name, app_utils::strutils::to_string(default_value), description)), \
                                true); \
    } \
  }; \
  var_type var_name = (var_name##_descriptor_t :: register_descriptor(*this), default_value)


template<typename CRTP>
struct ReflexioStructBase {
#ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
  using PybindClassType = pybind11::class_<CRTP>;
#endif

 private:
  using member_register_t = std::vector<std::unique_ptr<member_descriptor_t const>>;
  inline static std::vector<std::unique_ptr<member_descriptor_t const>> member_register;

 protected:
  static void register_member(std::unique_ptr<member_descriptor_t> member_descriptor) { 
    member_register.emplace_back(std::move(member_descriptor)); 
  }

 public:
  static member_register_t const& get_member_descriptors() { 
    static CRTP type; // forces population of member register once
    return member_register; 
  }
  
  static size_t num_members() { return get_member_descriptors().size(); }
};

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)  \
  struct StructName : ReflexioStructBase<StructName> {\
    using TypeName = StructName; \
    __VA_ARGS__;                                 \
  }

#ifdef REFLEXIO_STRUCT_USE_PYBIND_MODULE
namespace pybind11 {

template <typename ReflexioStruct, typename PyModule>
void wrap_reflexio_struct(PyModule& pymodule) {
  static std::string const typeName = app_utils::typeName<ReflexioStruct>();
  auto wrappedType = typename ReflexioStruct::PybindClassType(pymodule, typeName.c_str());
  wrappedType.def(pybind11::init<>());
  for (auto& member_descriptor : ReflexioStruct::get_member_descriptors()) {
    member_descriptor->wrap_in_pybind(&wrappedType);
  }
}
}  // namespace pybind11
#endif
