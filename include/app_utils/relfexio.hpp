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

  std::string get_name() const { return m_name; }
  std::string const& get_description() const { return m_description; }
  std::string default_as_string() const { return m_default_str; }

  /*
  virtual std::bytes value_as_bytes() const = 0;
  virtual set_value(std::bytes);
  virtual std::string value_as_string() const = 0;
  virtual set_value(std::string);
  */
};


template<typename MemberType>
struct member_descriptor_impl_t : public member_descriptor_t {

  using member_descriptor_t::member_descriptor_t;
  std::type_info const& get_member_type_info() const override { return typeid(MemberType); }
};

/*
#define REFLEXIO_STRUCT_DEFINE(StructName,
REFLEXIO_MEMBER_VAR_DEFINE(type1, member1, default_val1, description1);
REFLEXIO_MEMBER_VAR_DEFINE(type2, member2, default_val2, description2);
);
*/

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description) \
  struct var_name ## _descriptor {\
    static void register_descriptor(ReflexioStruct& reflexioStruct) { \
      static bool registered =                                                 \
          (                 \
          TypeName::register_member(                                                               \
              std::make_unique<member_descriptor_impl_t<var_type>>(#var_name, #default_value, #description)), \
          true); \
    } \
  }; \
  var_type var_name = (var_name##_descriptor :: register_descriptor(*this), default_value)


template<typename CRTP>
struct ReflexioStruct {
 private:
   using member_register_t = std::vector<std::unique_ptr<member_descriptor_t const>>;
  inline static std::vector<std::unique_ptr<member_descriptor_t const>> member_register;

 protected:
  static void register_member(std::unique_ptr<member_descriptor_t> member_descriptor) { 
    member_register.emplace_back(std::move(member_descriptor)); 
  }

 public:
  static size_t num_members() { return member_register.size(); }
};

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)  \
  struct StructName : ReflexioStruct<StructName> {           \
    using TypeName = StructName; \
    __VA_ARGS__;                                 \
  }
