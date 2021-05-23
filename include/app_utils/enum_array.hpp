#pragma once

#include "enumator.h"
#include <array>



namespace guille {
  /*
  * Array that can be indexed on enum values
  */

  template<typename T, typename EnumT>
  class EnumArray : public std::array<T, Enumator<EnumT>::size()>{

  public:
    using ArrayType = std::array<T, Enumator<EnumT>::size()>;

    // [c++17] TODO: that code can go away
    template<typename ...Args>
    EnumArray(Args&&... args) : ArrayType({ T(std::forward<Args>(args))... }) {}

    EnumArray(T defaultVal = {}) {
      this->fill(defaultVal);
    }

    using ArrayType::operator[];

    
    constexpr T const& operator[](EnumT enumVal) const {
      return operator[](static_cast<size_t>enumVal));

    }

    constexpr T& operator [](EnumT enumVal) {

      return operator[](static_cast<size_t>(enumVal));
    }

  }