#pragma once

#include <span>
#include <string>
#include <string_view>

namespace reflexio {

/*
  DefaultType: work around constexpr transient allocation for std::string.
  See reply there: https://stackoverflow.com/a/69590837/4249338
  Something similar should be done for e.g. std::vector.
*/
template<typename T>
struct reflexio_traits {
  using DefaultType = T;
  static constexpr std::span<T const> (*get_values)() = nullptr;
};

template <>
struct reflexio_traits<std::string> {
  using DefaultType = std::string_view;
  static constexpr std::span<std::string const> (*get_values)() = nullptr;
};


}// namespace reflexio