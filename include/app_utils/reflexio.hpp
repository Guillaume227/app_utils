#pragma once


#include <array>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include "reflexio_field_descriptor.hpp"
#include "reflexio_iterator.hpp"

#ifndef REFLEXIO_MINIMAL_FEATURES
#include <unordered_map>
#include <sstream>
#include <app_utils/string_utils.hpp>
#endif

namespace reflexio {

template <typename ReflexioStruct, size_t NumMemberVariables>
struct ReflexioStructBase {
  using ReflexioTypeName = ReflexioStruct;
#ifdef DO_PYBIND_WRAPPING
  using PybindClassType = pybind11::class_<ReflexioStruct>;
#endif

 protected:
  using member_var_register_t =
          std::array<member_descriptor_t<ReflexioStruct> const*, NumMemberVariables>;

 public:
  static constexpr size_t NumMemberVars = NumMemberVariables;
  static constexpr size_t num_registered_member_vars() {
    return NumMemberVariables; }

  using MemberVarsMask = std::bitset<NumMemberVariables>;

  template <typename T2>
  static constexpr size_t offset_of(T2 ReflexioStruct::* const member) {
    ReflexioStruct* object = nullptr;
    return size_t(&(object->*member)) - size_t(object);
  }

  template <typename T2>
  static size_t index_of_var(T2 ReflexioStruct::* const varPtr) {
    size_t offset = offset_of(varPtr);
    static const auto var_offsets = []{
      std::array<size_t, NumMemberVariables> offsets;
      for (size_t i = 0; i < NumMemberVariables; i++) {
        offsets[i] = ReflexioStruct::s_member_var_register[i]->get_var_offset();
      }
      return offsets;
    }();
    for (size_t i = 0; i < NumMemberVariables; i++) {
      if (var_offsets[i] == offset) {
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

  struct View {
    MemberVarsMask const& m_excludeMask;
    constexpr View(MemberVarsMask const& excludeMask)
        : m_excludeMask(excludeMask) {}

    using Iterator = ReflexioIterator<ReflexioStruct>;
    constexpr Iterator begin() const { return Iterator(0, m_excludeMask); }
    constexpr Iterator end  () const { return Iterator(NumMemberVars); }
  };

  constexpr static member_var_register_t const& get_member_descriptors() {
    return ReflexioStruct::s_member_var_register;
  }

  constexpr static View get_member_descriptors(MemberVarsMask const& excludeMask) {
    /*
    auto is_included =
            [&excludeMask, i=size_t{0}](member_descriptor_t const*) mutable {
                return not excludeMask.test(i++); };
    return ReflexioStruct::s_member_var_register | std::views::filter(is_included);
    */
    return {excludeMask};
  }

  constexpr ReflexioStruct const& cast_this() const {
    return static_cast<ReflexioStruct const&>(*this);
  }

  constexpr ReflexioStruct& cast_this() {
    return static_cast<ReflexioStruct&>(*this);
  }

#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  friend constexpr bool operator==(ReflexioStruct const& self, ReflexioStruct const& other) {
    for (auto& descriptor : self.get_member_descriptors()) {
      if (descriptor->values_differ(self, other)) {
        return false;
      }
    }
    return true;
  }

  [[nodiscard]]
  friend constexpr bool operator!=(ReflexioStruct const& self, ReflexioStruct const& other) {
    return not(self == other); }
#endif

#ifndef REFLEXIO_MINIMAL_FEATURES

  using Iterator = ReflexioIterator<ReflexioStruct>;
  constexpr Iterator begin() const { return Iterator(0); }
  constexpr Iterator end  () const { return Iterator(NumMemberVars); }

  [[nodiscard]]
  constexpr bool has_all_default_values(
          MemberVarsMask const& excludeMask={}) const
  {
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(cast_this())) {
        return false;
      }
    }
    return true;
  }

  constexpr void set_to_default(
          MemberVarsMask const& excludeMask={}) {
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      descriptor.set_to_default(cast_this());
    }
  }

  [[nodiscard]]
  std::vector<std::string_view> non_default_values(
          MemberVarsMask const& excludeMask={}) const
  {
    std::vector<std::string_view> res;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(cast_this())) {
        res.push_back(descriptor.get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::vector<std::string_view> differing_members(
          ReflexioStruct const& other,
          MemberVarsMask const& excludeMask={}) const
  {
    std::vector<std::string_view> res;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (descriptor.values_differ(cast_this(), other)) {
        res.push_back(descriptor.get_name());
      }
    }
    return res;
  }

  [[nodiscard]]
  std::string differences(
          ReflexioStruct const& other,
          MemberVarsMask const& excludeMask={}) const {
    std::ostringstream out;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (descriptor.values_differ(cast_this(), other)) {
        out << descriptor.get_name() << ": ";
        descriptor.value_to_yaml(cast_this(), out);
        out << " vs ";
        descriptor.value_to_yaml(other.cast_this(), out);
        out << '\n';
      }
    }
    return out.str();
  }

  constexpr friend std::ostream& to_yaml(
          ReflexioStruct const& instance,
          std::ostream& os)
  {
    yaml_utils::indenter_t indenter;
    bool const am_i_nested = yaml_utils::get_indent_depth() > 0;
    if (am_i_nested) {
      os << "\n";
    }
    auto const descriptors = ReflexioStruct::get_member_descriptors();
    for (size_t i = 0; i < descriptors.size(); i++){
      auto& descriptor = descriptors[i];
      yaml_utils::print_indent(os);
      os << descriptor->get_name() << ": ";
      descriptor->value_to_yaml(instance, os);
      if (i < descriptors.size() - 1 or not am_i_nested) {
        os << '\n'; // avoid adding a newline
      }
    }
    return os;
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  ReflexioStruct const& instance) {
    to_yaml(instance, os);
    return os;
  }

  [[nodiscard]]
  friend std::string to_yaml(
        ReflexioStruct const& instance) {
    std::ostringstream oss;
    oss << instance;
    return oss.str();
  }

  friend std::istream& from_yaml(
          ReflexioStruct& instance,
          std::istream& is)
  {

    using descriptor_map_t = std::unordered_map<std::string_view, member_descriptor_t<ReflexioStruct> const*>;
    static const descriptor_map_t descriptor_map =
            []{
              descriptor_map_t map;
              for (auto& descriptor: ReflexioStruct::get_member_descriptors()) {
                map[descriptor->get_name()] = descriptor;
              }
              return map;
            }();

    bool first_line_seen = false;
    size_t start_indent = 0;

    for(std::string raw_line; std::getline(is, raw_line);) {

      std::string_view line = app_utils::strutils::strip(raw_line);
      size_t indent = raw_line.find_first_not_of(' ') / yaml_utils::indent_width;
      if (line == "---") {
        if (first_line_seen) {
          // signals the start of another section
          break;
        } else {
          checkCond(indent == 0);
          start_indent = 0;
        }
        continue;
      } else if (line == "...") {
        // end of section
        break;
      } else if (not first_line_seen) {
        start_indent = indent;
      } else {
        if (start_indent > indent) {
          int rewind_offset = -static_cast<int>(raw_line.size()) - 1;
          is.seekg(rewind_offset, std::ios_base::cur);
          break;
        }
      }

      first_line_seen = true;

      size_t separator_pos = line.find_first_of(':');

      checkCond(separator_pos != std::string::npos, "bad name value pair:", raw_line);
      auto const name = line.substr(0, separator_pos);
      auto const val = line.substr(separator_pos + 1);
      auto it = descriptor_map.find(name);
      checkCond(it != descriptor_map.end(), "unrecognized",
                app_utils::typeName<ReflexioStruct>(), "member name", name);

      if (val.empty()) {
        it->second->set_value_from_yaml(instance, is);
      } else {
        // value fits on just one line
        try {
          int rewind_offset = -static_cast<int>(raw_line.size() - raw_line.find_first_of(':') - 1);
          is.seekg(rewind_offset, std::ios_base::cur);
          it->second->set_value_from_yaml(instance, is);
        } catch (std::exception const& exc) {
          throwExc("Failed parsing yaml line:", raw_line, exc.what());
        }
      }
    }

    return is;
  }

  friend void from_yaml(
          ReflexioStruct& instance,
          std::string_view const val_str) {
    instance.set_to_default();
    std::istringstream iss (std::string{val_str});
    from_yaml(instance, iss);
  }

  friend std::istream& operator>>(std::istream& is,
                                  ReflexioStruct const& instance) {
    from_yaml(is, instance);
    return is;
  }

  static std::string const& get_docstring(MemberVarsMask const& excludeMask={}) {
    static std::string const docstring = [&excludeMask] {
      std::ostringstream oss;
      for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
        oss << descriptor.get_name() << ": "
            << descriptor.get_description() << "\n";
      }
      return oss.str();
    }();
    return docstring;
  }

#endif

  [[nodiscard]]
  friend constexpr size_t serial_size(
          ReflexioStruct const& val) {
    return val.get_serial_size();
  }

  [[nodiscard]]
  friend constexpr size_t serial_size(
          ReflexioStruct const& val,
          MemberVarsMask const& excludeMask) {
    return val.get_serial_size(excludeMask);
  }

  [[nodiscard]]
  constexpr size_t get_serial_size(MemberVarsMask const& excludeMask) const {
    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      res += descriptor.get_serial_size(cast_this());
    }
    return res;
  }

  [[nodiscard]]
  constexpr size_t get_serial_size() const {
    size_t res = 0;
    for (auto& descriptor : ReflexioStruct::get_member_descriptors()) {
      res += descriptor->get_serial_size(cast_this());
    }
    return res;
  }

  // return number of written bytes
  friend size_t to_bytes(std::byte* buffer,
                         size_t const buffer_size,
                         ReflexioStruct const& instance,
                         MemberVarsMask const& excludeMask={}) {
    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      res += descriptor.write_to_bytes(buffer + res, buffer_size - res, instance);
    }
    checkCond(buffer_size >= res, "output buffer is not big enough to fit object", buffer_size, '<', res);
    return res;
  }

  friend size_t to_bytes(std::span<std::byte> buffer,
                         ReflexioStruct& instance,
                         MemberVarsMask const& excludeMask={}) {
    return to_bytes(buffer.data(), buffer.size(), instance, excludeMask);
  }

  // return number of bytes read
  friend size_t from_bytes(std::byte const* buffer,
                           size_t const buffer_size,
                           ReflexioStruct& instance,
                           MemberVarsMask const& excludeMask={}) {
    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      res += descriptor.read_from_bytes(buffer + res, buffer_size - res, instance);
    }
    checkCond(buffer_size >= res, "input buffer has less data than required:", buffer_size, '<', res, 
      ". Look for inconsistent serialization/deserialization of", app_utils::typeName<ReflexioStruct>());
    return res; //TODO: revisit, saw mismatch between buffer size (383) and read byte (386) buffer_size >= res ? res : 0;
  }

  friend size_t from_bytes(std::span<std::byte const> buffer,
                           ReflexioStruct& instance,
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

}  // namespace reflexio

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description)         \
  var_type var_name = var_type(default_value);                                             \
                                                                                           \
  inline static constexpr auto __##var_name##_descr = [] {                                 \
    return reflexio::member_descriptor_impl_t<ReflexioTypeName, var_type>{                 \
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
    static constexpr                                                                       \
    reflexio::member_descriptor_t<ReflexioTypeName> const* descriptor =                    \
            &__##var_name##_descr;                                                         \
  }

// define a member variable with a 'default default'
#define REFLEXIO_MEMBER_VAR_DEFINE_DEF(var_type, var_name, description) \
  REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, var_type(), description)

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)                                         \
  struct StructName                                                                     \
        : reflexio::ReflexioStructBase<                                                 \
              StructName,                                                               \
              reflexio::count_member_var_declarations(#__VA_ARGS__)> {                  \
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
    __VA_ARGS__ /* member variables are injected here */                                \
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
