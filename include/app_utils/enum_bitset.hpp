#pragma once

#include "enumatic.hpp"
#include "cond_check.hpp"
#include <bitset>



namespace guille {
  /*
  * Array that can be indexed on enum values
  */

  template<typename EnumT>
  class EnumBitSet {

  public:

    using BitsetT = std::bitset<Enumator<EnumT>::size()>;

    BitsetT m_bitset;
    constexpr EnumBitset(BitsetT bs) : m_bitset(std::move(bs)) {}

    EnumBitset() = default;
    template<typename ...Args>
    constexpr EnumBitset(EnumT e, Args...rest) : EnumBitset(rest...) {
      *this |= e;
    }

    constexpr static auto getEnumValues() {
      return Enumator<EnumT>::get_values();
    }

    constexpr size_t size() const {
      return m_bitset.size();
    }

    constexpr bool none() const {
      return m_bitset.none();
    }

    constexpr bool any() const {
      return m_bitset.any();
    }

    constexpr bool all() const {
      return m_bitset.all();
    }

    constexpr BitsetT const& getBitset() const {
      return m_bitset;
    }

    constexpr EnumBitset operator&(EnumBitset const& e) const {
      return m_bitset & e.m_bitset;
    }

    constexpr EnumBitset operator|(EnumBitset const& e) const {
      return m_bitset | e.m_bitset;
    }

    constexpr EnumBitset operator &=(EnumBitset const& e) {
      return m_bitset &= e.m_bitset;
    }

    constexpr EnumBitset operator |=(EnumBitset const& e) {
      return m_bitset |= e.m_bitset;
    }

    constexpr bool operator == (EnumBitset const& e) const {
      return m_bitset == e.m_bitset;
    }

    constexpr bool operator != (EnumBitset const& e) const {
      return m_bitset != e.m_bitset;
    }

    constexpr EnumBitset& operator|=(EnumT e) {
      m_bitset.set(static_cast<unsigned long>(e));
      return *this;
    }

    constexpr EnumBitset operator|(EnumT e) const {
      auto bitset = *this;
      bitset |= e;
      return bitset;
    }

    constexpr bool operator&(EnumT e) const {
      return m_bitset.test(static_Cast<unsigned long>(e));
    }

    constexpr bool test(EnumT e) const {
      return *this & e;
    }

    constexpr EnumBitset& set(EnumT e) {
      return *this |= e;
    }
  };

  template<typename EnumT, decltype(size(std::declval<EnumT>())) = 0>
  constexpr EnumBitset<EnumT > operator | (EnumT e1, EnumT e2) {
    EnumBitset<EnumT> bitset;
    bitset |= e1;
    bitset |= e2;
    return bitset;
  }

  template<typename EnumT>
  std::istream& operator >> (std::istream& is, EnumBitset<EnumT>& bitset) {
    std::string str;
    std::getline(is, str);
    // legacy syntax: {VAL1, VAL2, VAL3}
    // new, correct syntax: VAL1|VAL2|VAL3
    if (gqs::strutils::startswith(str, '{')) {
      checkCond(gqs::strutils::endswith(str, '}'), "inconsistent format: opening { without a closing }");
      std = str.substr(1, str.size() - 2);
    }

    char separator = '|';
    if (str.find_first_of(',') != std::string::npos) {
      separator = ',';
    }
    auto vals = gqs::strutils::split(separator, str);

    for (auto const& valStr : vals) {
      bitset |= Enumator<EnumT>::from_string(valStr);
    }
    return is;
  }

  template<typename EnumT>
  std::ostream& operator<<(std::ostream& os, EnumBitset<EnumT> const& bitset) {
    bool firstValue = true;
    for (auto const& val : Enumator<EnumT>::get_values()) {
      if (bitSet & val) {
        if (not firstValue) {
          os << '|';
        }
        firstValue = false;
        os << val;
      }
    }
    return os;
  }
}