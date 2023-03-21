#pragma once


#include <array>
#include <bitset>
#include <cstddef> // std::byte
#include <utility>
#include "reflexio_field_descriptor.hpp"
#include "reflexio_iterator.hpp"
#include "reflexio_view.hpp"

#ifndef REFLEXIO_MINIMAL_FEATURES
#include "reflexio_utils.hpp"
#include <stdexcept>
#endif

namespace reflexio {
#ifndef REFLEXIO_MINIMAL_FEATURES
class DeserializationException : public app_utils::Exception {
  using app_utils::Exception::Exception;
};
class PartialDeserializationException : public DeserializationException {
  using DeserializationException::DeserializationException;
};
class CorruptedDeserializationException : public DeserializationException {
  using DeserializationException::DeserializationException;
};
#endif

template <typename ReflexioStruct, size_t NumMemberVariables>
struct ReflexioStructBase {
  using ReflexioTypeName = ReflexioStruct;
#ifdef DO_PYBIND_WRAPPING
  using PybindClassType = pybind11::class_<ReflexioStruct>;
#endif

 protected:
  using member_var_register_t =
          std::array<member_descriptor_t const*, NumMemberVariables>;

 public:
  static constexpr size_t NumMemberVars = NumMemberVariables;
  static constexpr size_t num_registered_member_vars() {
    return NumMemberVariables;
  }

  using Mask = std::bitset<NumMemberVariables>;
  using MemberVarsMask = Mask;
  static constexpr Mask exclude_none = {};

  using View = reflexio_view<ReflexioStruct>;
  using ConstView = reflexio_view<ReflexioStruct const>;
  using FatView = reflexio_fat_view<ReflexioStruct>;

  // same as:
  // offsetof(ReflexioStruct, member);
  // but can be called in a programmatic way.
  template <typename T2>
  static constexpr size_t offset_of(T2 ReflexioStruct::* const member) {
    ReflexioStruct* object = nullptr;
    return size_t(&(object->*member)) - size_t(object);
  }

  static constexpr auto get_member_var_offsets() {
    std::array<size_t, NumMemberVariables> offsets;
    for (size_t i = 0; i < NumMemberVariables; i++) {
      offsets[i] = ReflexioStruct::s_member_var_register[i]->get_var_offset();
    }
    return offsets;
  }

  template <typename T2>
  static size_t index_of_var(T2 ReflexioStruct::* const varPtr) {
    // do not use static as it leads to extra code size in embedded setting (gcc)
    //static const auto member_var_offsets = get_member_var_offsets();
    size_t offset = offset_of(varPtr);
    for (size_t i = 0; i < NumMemberVariables; i++) {
      if (ReflexioStruct::s_member_var_register[i]->get_var_offset() == offset) {
        return i;
      }
    }
    return 0; // should never get there - how to enforce it during compilation?
  }

  template <typename Arg, typename... Args>
  static /*consteval*/ bool strictly_increasing(Arg const& arg0, Args const&... args) {
    if constexpr (sizeof...(args) == 0) {
      (void) arg0; // avoid unreferenced formal parameter compiler warning
      return true;
    } else {
      auto arg = index_of_var(arg0);
      return ((arg < index_of_var(args) ? (arg = index_of_var(args), true) : false) && ...);
    }
  }

  template<typename ...VarPtrs>
    requires(sizeof...(VarPtrs) <= NumMemberVariables)
  constexpr static Mask make_vars_mask(VarPtrs const& ... varPtrs) {
    // strictly_increasing can't be called in a constexpr context (because of index_of_var)
    //static_assert(strictly_increasing(varPtrs...));
    Mask include_mask;
    (include_mask.set(index_of_var(varPtrs)), ...);
    return include_mask.flip();
  }

  struct MembersDescriptorView {
    Mask const& m_excludeMask;
    constexpr MembersDescriptorView(Mask const& excludeMask)
        : m_excludeMask(excludeMask) {}

    using Iterator = ReflexioIterator<ReflexioStruct>;
    constexpr Iterator begin() const { return Iterator(0, m_excludeMask); }
    constexpr Iterator end  () const { return Iterator(NumMemberVars); }
  };

  constexpr static member_var_register_t const& get_member_descriptors() {
    return ReflexioStruct::s_member_var_register;
  }

  constexpr static MembersDescriptorView get_member_descriptors(Mask const& excludeMask) {
    /*
    auto is_included =
            [&excludeMask, i=size_t{0}](member_descriptor_t const*) mutable {
                return not excludeMask.test(i++); };
    return ReflexioStruct::s_member_var_register | std::views::filter(is_included);
    */
    return {excludeMask};
  }

#ifndef REFLEXIO_NO_COMPARISON_OPERATORS
  [[nodiscard]]
  friend constexpr bool operator==(ReflexioStruct const& self, ReflexioStruct const& other) {
    for (auto& descriptor : self.get_member_descriptors()) {
      if (descriptor->values_differ(&self, &other)) {
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
          Mask const& excludeMask=exclude_none) const
  {
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(this)) {
        return false;
      }
    }
    return true;
  }

  constexpr void set_to_default(
          Mask const& excludeMask=exclude_none) {
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      descriptor.set_to_default(this);
    }
  }

  [[nodiscard]]
  std::vector<std::string_view> non_default_values(
          Mask const& excludeMask=exclude_none) const
  {
    std::vector<std::string_view> res;
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (not descriptor.is_at_default(this)) {
        res.push_back(descriptor.get_name());
      }
    }
    return res;
  }

  /**
   *
   * @param other
   * @param excludeMask fields to leave out of the diff
   * @return a mask that is false at indexes that differ (among the fields not excluded by excludeMask)
   */
  [[nodiscard]]
  Mask matching_fields(ReflexioStruct const& other,
                       Mask const& excludeMask=exclude_none) const {
    Mask res;
    res.flip();
    for (auto& descriptor: get_member_descriptors(excludeMask)) {
      if (descriptor.values_differ(this, &other)) {
        res.set(descriptor.get_index(), false);
      }
    }
    return res;
  }

  [[nodiscard]]
  std::vector<std::string_view> differing_members(
          ReflexioStruct const& other,
          Mask const& excludeMask=exclude_none) const
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
          ReflexioStruct const& other,
          Mask const& exclude_mask=exclude_none) const {
    return details::differences(
            this,
            &other,
            ReflexioStruct::get_member_descriptors(),
            [&exclude_mask](size_t idx){ return exclude_mask.test(idx); });
  }

  constexpr friend std::ostream& to_yaml(
          ConstView const& view,
          std::ostream& os)
  {
    return details::to_yaml(
            &view.object,
            ReflexioStruct::get_member_descriptors(),
            [&view](size_t idx){ return view.exclude_mask.test(idx); },
            os);
  }

  constexpr friend std::ostream& to_yaml(
          ReflexioStruct const& obj,
          std::ostream& os) {
    return to_yaml(ConstView{obj}, os);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  ConstView const& instance) {
    to_yaml(instance, os);
    return os;
  }

  [[nodiscard]]
  friend std::string to_yaml(
        ConstView const& instance) {
    std::ostringstream oss;
    oss << instance;
    return oss.str();
  }

  [[nodiscard]]
  friend std::string to_string(
          ConstView const& instance) {
    return to_yaml(instance);
  }

  friend std::istream& from_yaml(
          ReflexioStruct& instance,
          std::istream& is,
          Mask const& exclude_mask=exclude_none,
          int const line_offset = 0)
  {
    return details::from_yaml(
            &instance,
            ReflexioStruct::get_member_descriptors(),
            is,
            [&exclude_mask](size_t idx){ return exclude_mask.test(idx); },
            line_offset);
  }

  friend std::istream& from_yaml(
          View& instance,
          std::istream& is) {
    return from_yaml(instance.object, is, instance.exclude_mask);
  }

  friend void from_yaml(
          ReflexioStruct& instance,
          std::string_view const val_str,
          Mask const& exclude_mask=exclude_none) {
    std::istringstream iss (std::string{val_str});
    from_yaml(instance, iss, exclude_mask);
  }

  friend void from_yaml(
          View& instance,
          std::string_view const val_str) {
    from_yaml(instance.object, val_str, instance.exclude_mask);
  }

  friend void from_string(
        ReflexioStruct& instance,
        std::string_view const val_str,
        Mask const& exclude_mask=exclude_none) {
    from_yaml(instance, val_str, exclude_mask);
  }

  friend void from_string(
          View& instance,
          std::string_view const val_str) {
    from_string(instance.object, val_str, instance.exclude_mask);
  }

  friend std::istream& operator>>(std::istream& is,
                                  ReflexioStruct const& instance) {
    from_yaml(is, instance);
    return is;
  }

  static std::string const& get_docstring() {
    static auto const doc_string = details::get_docstring(ReflexioStruct::get_member_descriptors());
    return doc_string;
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
          Mask const& excludeMask) {
    return val.get_serial_size(excludeMask);
  }

  [[nodiscard]]
  constexpr size_t get_serial_size(Mask const& excludeMask) const {
    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      res += descriptor.get_serial_size(this);
    }
    return res;
  }

  [[nodiscard]]
  constexpr size_t get_serial_size() const {
    size_t res = 0;
    for (auto& descriptor : ReflexioStruct::get_member_descriptors()) {
      res += descriptor->get_serial_size(this);
    }
    return res;
  }

  // return number of written bytes
  friend size_t to_bytes(std::byte* buffer,
                         size_t const buffer_size,
                         ReflexioStruct const& instance,
                         Mask const& excludeMask=exclude_none) {
    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      //std::cout << "    encoded " << descriptor.get_name() << std::endl;
      res += descriptor.write_to_bytes(buffer + res, buffer_size - res, &instance);
    }
    //std::cout << std::endl;
    checkCond(buffer_size >= res, "output buffer is not big enough to accommodate object", buffer_size, '<', res);
    return res;
  }

  friend size_t to_bytes(std::span<std::byte> buffer,
                         ReflexioStruct& instance,
                         Mask const& excludeMask=exclude_none) {
    return to_bytes(buffer.data(), buffer.size(), instance, excludeMask);
  }

  // return number of bytes read
  friend size_t from_bytes(std::byte const* buffer,
                           size_t const buffer_size,
                           ReflexioStruct& instance,
                           Mask const& excludeMask=exclude_none) {

    size_t res = 0;
    for (auto& descriptor: ReflexioStruct::get_member_descriptors(excludeMask)) {
      if (buffer_size <= res) {
#ifdef RTTI_ENABLED
        throwWithTrace(PartialDeserializationException,
                       descriptor.get_name(), ": no data left for deserialization of", app_utils::typeName<ReflexioStruct>());
#else
        return res;
#endif
      }

      res += descriptor.read_from_bytes(buffer + res, buffer_size - res, &instance);
#ifdef RTTI_ENABLED
      if (buffer_size < res) {
        throwWithTrace(CorruptedDeserializationException, descriptor.get_name(), ": not enough data for deserialization of",
                  app_utils::typeName<ReflexioStruct>(), "required", res, "bytes, found", buffer_size);
      }
#endif
    }

    return res;
  }

  friend size_t from_bytes(std::span<std::byte const> buffer,
                           ReflexioStruct& instance,
                           Mask const& excludeMask=exclude_none) {
    return from_bytes(buffer.data(), buffer.size(), instance, excludeMask);
  }
};

namespace details {
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
}

template <typename T>
using is_reflexio_struct = std::is_base_of<ReflexioStructBase<T, T::NumMemberVars>, T>;

} // namespace reflexio

#define _REFLEXIO_MEMBER_VAR_DEFINE_BOUND_FUNC(var_type, var_name, default_value, description, boundMinFunc, boundMaxFunc) \
  var_type var_name = var_type(default_value);                                             \
  static constexpr int __##var_name##_id = __COUNTER__;                                    \
  template<class Dummy>                                                                    \
  struct member_var_counter_t<__##var_name##_id, Dummy> {                                  \
    static constexpr int index =                                                           \
            member_var_counter_t<__##var_name##_id - 1, Dummy>::index + 1;                 \
  };                                                                                       \
                                                                                           \
  inline static constexpr auto __##var_name##_descr = [] {                                 \
    return reflexio::member_descriptor_impl_t<ReflexioTypeName, var_type>(                 \
            &ReflexioTypeName::var_name,                                                   \
            (size_t)member_var_counter_t<__##var_name##_id, int>::index,                   \
            #var_name,                                                                     \
            description,                                                                   \
            default_value,                                                                 \
            boundMinFunc,                                                                  \
            boundMaxFunc);                                                                 \
  }();                                                                                     \
                                                                                           \
                                                                                           \
  template<class Dummy>                                                                    \
  struct member_var_traits_t<member_var_counter_t<__##var_name##_id, int>::index, Dummy> { \
    static constexpr                                                                       \
    reflexio::member_descriptor_t const* descriptor = &__##var_name##_descr;               \
  }

#define REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, default_value, description) \
  _REFLEXIO_MEMBER_VAR_DEFINE_BOUND_FUNC(var_type, var_name, default_value, description, nullptr, nullptr)

#define REFLEXIO_MEMBER_VAR_DEFINE_MIN_MAX(var_type, var_name, default_value, description, minBound, maxBound) \
  _REFLEXIO_MEMBER_VAR_DEFINE_BOUND_FUNC(var_type, var_name, default_value, description,                       \
    []()->var_type const&{static var_type val = minBound; return val; },                                       \
    []()->var_type const&{static var_type val = maxBound; return val; })

#define REFLEXIO_MEMBER_VAR_DEFINE_MIN(var_type, var_name, default_value, description, minBound) \
  _REFLEXIO_MEMBER_VAR_DEFINE_BOUND_FUNC(var_type, var_name, default_value, description,         \
    []()->var_type const&{static var_type val = minBound; return val; }, nullptr)

#define REFLEXIO_MEMBER_VAR_DEFINE_MAX(var_type, var_name, default_value, description, maxBound) \
  _REFLEXIO_MEMBER_VAR_DEFINE_BOUND_FUNC(var_type, var_name, default_value, description,         \
    nullptr, []()->var_type const&{static var_type val = maxBound; return val; })

// define a member variable with a 'default default'
#define REFLEXIO_MEMBER_VAR_DEFINE_DEF(var_type, var_name, description) \
  REFLEXIO_MEMBER_VAR_DEFINE(var_type, var_name, var_type(), description)

#define REFLEXIO_STRUCT_DEFINE(StructName, ...)                                         \
  struct StructName                                                                     \
        : reflexio::ReflexioStructBase<                                                 \
              StructName,                                                               \
              reflexio::details::count_member_var_declarations(#__VA_ARGS__)> {         \
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
