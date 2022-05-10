#pragma once


#include <array>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include "reflexio_field_descriptor.hpp"
#include "reflexio_iterator.hpp"

namespace app_utils::reflexio {

template <typename CRTP, size_t NumMemberVariables>
struct ReflexioStructBase {
  using ReflexioTypeName = CRTP;
#ifdef DO_PYBIND_WRAPPING
  using PybindClassType = pybind11::class_<CRTP>;
#endif

 protected:
  using member_var_register_t =
          std::array<member_descriptor_t const*, NumMemberVariables>;

 public:
  static constexpr size_t NumMemberVars = NumMemberVariables;
  static constexpr size_t num_registered_member_vars() {
    return NumMemberVariables; }

  using MemberVarsMask = std::bitset<NumMemberVariables>;

#ifndef REFLEXIO_MINIMAL_FEATURES
  template <typename T2>
  static constexpr size_t offset_of(T2 CRTP::* const member) {
    CRTP object {};
    return size_t(&(object.*member)) - size_t(&object);
  }

  template <typename T2>
  static constexpr size_t index_of_var(T2 CRTP::* const varPtr) {
    size_t offset = offset_of(varPtr);
    for (size_t i = 0; i < NumMemberVariables; i++) {
      if (CRTP::s_member_var_register[i]->get_var_offset() == offset) {
        return i;
      }
    }
    return 0; // should never get there - how to enforce it during compilation?
  }

  template<typename ...VarPtrs>
    requires(sizeof...(VarPtrs) <= NumMemberVariables)
  constexpr static MemberVarsMask make_vars_mask(VarPtrs const& ... varPtrs) {
    MemberVarsMask include_mask;
    (include_mask.set(index_of_var(varPtrs)), ...);
    return include_mask.flip();
  }
#endif

  struct View {
    MemberVarsMask const& m_excludeMask;
    constexpr View(MemberVarsMask const& excludeMask)
        : m_excludeMask(excludeMask) {}

    using Iterator = ReflexioIterator<CRTP>;
    constexpr Iterator begin() const { return Iterator(0, m_excludeMask); }
    constexpr Iterator end  () const { return Iterator(NumMemberVars); }
  };

  constexpr static member_var_register_t const& get_member_descriptors() {
    return CRTP::s_member_var_register;    
  }

  constexpr static View get_member_descriptors(MemberVarsMask const& excludeMask) {
    /*
    auto is_included =
            [&excludeMask, i=size_t{0}](member_descriptor_t const*) mutable {
                return not excludeMask.test(i++); };
    return CRTP::s_member_var_register | std::views::filter(is_included);
    */
    return {excludeMask};
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
  constexpr Iterator begin() const { return Iterator(0); }
  constexpr Iterator end  () const { return Iterator(NumMemberVars); }

  [[nodiscard]]
  constexpr bool has_all_default_values(
          MemberVarsMask const& excludeMask={}) const
  {
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(this)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]]
  std::vector<std::string_view> non_default_values(
          MemberVarsMask const& excludeMask={}) const
  {
    std::vector<std::string_view> res;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(this)) {
        res.push_back(descriptor.get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::vector<std::string_view> differing_members(
          CRTP const& other,
          MemberVarsMask const& excludeMask={}) const
  {
    std::vector<std::string_view> res;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (descriptor.values_differ(this, &other)) {
        res.push_back(descriptor.get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::string differences(
          CRTP const& other,
          MemberVarsMask const& excludeMask={}) const {
    std::ostringstream out;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (descriptor.values_differ(this, &other)) {
        out << descriptor.get_name() << ": "
            << descriptor.value_as_string(this) << " vs "
            << descriptor.value_as_string(&other) << '\n';
      }
    }
    return out.str();
  }

  [[nodiscard]]
  constexpr friend std::string to_string(
          CRTP const& instance,
          MemberVarsMask const& excludeMask={})
  {
    std::ostringstream oss;
    for (auto& descriptor: CRTP::get_member_descriptors(excludeMask)) {
      oss << descriptor.get_name() << ": " << descriptor.value_as_string(&instance) << "\n";
    }
    return oss.str();
  }

  static std::string const& get_docstring(MemberVarsMask const& excludeMask={}) {
    static std::string const docstring = [&excludeMask] {
      std::ostringstream oss;
      for (auto& descriptor: CRTP::get_member_descriptors(excludeMask)) {
        oss << descriptor.get_name() << ": " << descriptor.get_description() << "\n";
      }
      return oss.str();
    }();
    return docstring;
  }

#endif

  [[nodiscard]]
  friend constexpr size_t serial_size(
          CRTP const& val) {
    return val.get_serial_size();
  }

  [[nodiscard]]
  friend constexpr size_t serial_size(
          CRTP const& val,
          MemberVarsMask const& excludeMask) {
    return val.get_serial_size(excludeMask);
  }

  [[nodiscard]]
  constexpr size_t get_serial_size(MemberVarsMask const& excludeMask) const {
    size_t res = 0;
    for (auto& descriptor: CRTP::get_member_descriptors(excludeMask)) {
      res += descriptor.get_serial_size(this);
    }
    return res;
  }

  [[nodiscard]]
  constexpr size_t get_serial_size() const {
    size_t res = 0;
    for (auto& descriptor : CRTP::get_member_descriptors()) {
      res += descriptor->get_serial_size(this);
    }
    return res;
  }

  // return number of written bytes
  friend size_t to_bytes(std::byte* buffer,
                         size_t const buffer_size,
                         CRTP const& instance,
                         MemberVarsMask const& excludeMask={}) {
    size_t res = 0;
    for (auto& descriptor: CRTP::get_member_descriptors(excludeMask)) {
      res += descriptor.write_to_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "output buffer is not big enough to fit object", buffer_size, '<', res);
    return res;
  }

  friend size_t to_bytes(std::span<std::byte> buffer,
                         CRTP& instance,
                         MemberVarsMask const& excludeMask={}) {
    return to_bytes(buffer.data(), buffer.size(), instance, excludeMask);
  }

  // return number of bytes read
  friend size_t from_bytes(std::byte const* buffer,
                           size_t const buffer_size,
                           CRTP& instance,
                           MemberVarsMask const& excludeMask={}) {
    size_t res = 0;
    for (auto& descriptor: CRTP::get_member_descriptors(excludeMask)) {
      res += descriptor.read_from_bytes(buffer + res, buffer_size - res, &instance);
    }
    checkCond(buffer_size >= res, "input buffer has less data than required:", buffer_size, '<', res, 
      ". Look for inconsistent serialization/deserialization of", app_utils::typeName<CRTP>());
    return res; //TODO: revisit, saw mismatch between buffer size (383) and read byte (386) buffer_size >= res ? res : 0;
  }

  friend size_t from_bytes(std::span<std::byte const> buffer,
                           CRTP& instance,
                           MemberVarsMask const& excludeMask={}) {
    return from_bytes(buffer.data(), buffer.size(), instance, excludeMask);
  }
};

consteval size_t count_member_var_declarations(std::string_view const text) {
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

}  // namespace app_utils::reflexio

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description)         \
  var_type var_name = var_type(default_value);                                             \
                                                                                           \
  inline static constexpr auto __##var_name##_descr = [] {                                 \
    return app_utils::reflexio::member_descriptor_impl_t<var_type, ReflexioTypeName>{      \
            &ReflexioTypeName::var_name,                                                   \
            default_value,                                                                 \
            #var_name,                                                                     \
            description};                                                                  \
  }();                                                                                     \
                                                                                           \
  static constexpr int __##var_name##_id = __COUNTER__;                                    \
                                                                                           \
  template<class Dummy>                                                                    \
  struct member_var_counter_t<__##var_name##_id, Dummy> {                                  \
    static constexpr int index =                                                           \
            member_var_counter_t<__##var_name##_id - 1, Dummy>::index + 1;                 \
  };                                                                                       \
                                                                                           \
  template<class Dummy>                                                                    \
  struct member_var_traits_t<member_var_counter_t<__##var_name##_id, int>::index, Dummy> { \
    static constexpr app_utils::reflexio::member_descriptor_t const* descriptor =          \
            &__##var_name##_descr;                                                         \
  }

// define a member variable with a 'default default'
#define REFLEXIO_MEMBER_VAR_DEFINE_DEF(var_type, var_name, description) \
  REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, var_type(), description)

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)                                         \
  struct StructName                                                                     \
      : app_utils::reflexio::ReflexioStructBase<                                        \
              StructName,                                                               \
              app_utils::reflexio::count_member_var_declarations(#__VA_ARGS__)> {       \
                                                                                        \
    template<size_t N, class dummy>                                                     \
    struct member_var_traits_t {};                                                      \
                                                                                        \
    template<int N, class Dummy = int>                                                  \
    struct member_var_counter_t {                                                       \
      static constexpr int index = member_var_counter_t<N - 1, Dummy>::index;           \
    };                                                                                  \
                                                                                        \
    template<class Dummy>                                                               \
    struct member_var_counter_t<-1, Dummy> {                                            \
      static constexpr int index = -1;                                                  \
    };                                                                                  \
                                                                                        \
    constexpr StructName() = default;                                                   \
                                                                                        \
    __VA_ARGS__                                                                         \
                                                                                        \
    inline static constexpr member_var_register_t s_member_var_register =               \
            []<size_t... NN>(std::index_sequence<NN...>) {                              \
      member_var_register_t out{nullptr};                                               \
      std::size_t i = 0;                                                                \
      (void(out[i++] = member_var_traits_t<NN, int>::descriptor), ...);                 \
      return out;                                                                       \
    }                                                                                   \
    (std::make_index_sequence<num_registered_member_vars()>());                         \
  }
