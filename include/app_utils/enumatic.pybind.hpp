#pragma once

#include <app_utils/pybind_utils.hpp>
#include <app_utils/enumatic.hpp>

namespace app_utils::pybind {
template <typename EnumaticT>
struct pybind_wrapper<EnumaticT, std::enable_if_t<enumatic::is_enumatic_type<EnumaticT>(), int>> {
  inline static bool s_was_registered = false;
  template <class PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {
    if (not s_was_registered) {
      s_was_registered = true;
      static std::string const enum_name {Enumatic<EnumaticT>::name()};
      pybind11::enum_<EnumaticT> wrappedEnum(pybindHost, enum_name.c_str());
      // copy string_views into strings because pybind takes in a \0 terminated char const*
      static std::array<std::string, Enumatic<EnumaticT>::size()> string_values;
      int i = 0;
      for (auto value: Enumatic<EnumaticT>::get_values()) {
        string_values[i] = std::string{Enumatic<EnumaticT>::to_string(value)};
        wrappedEnum.value(string_values[i++].c_str(), value);
      }
    }
  }
};
}
