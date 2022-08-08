#pragma once

#include <span>
#include <string_view>

#include "reflexio_traits.hpp"
#include "enumatic.hpp"

namespace reflexio {

template<enumatic::EnumaticType T>
struct reflexio_traits<T> {
  using DefaultType = T;
  static constexpr std::span<std::string_view const> (*get_values)() = Enumatic<T>::get_values_str;
};
}// namespace reflexio

#include "reflexio.hpp"