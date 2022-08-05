#pragma once

#include <array>
#include <bitset>
#include <cstddef>// std::byte
#include <utility>


namespace reflexio {

/**
 * class for iterating over field descriptors
 */
template<typename ReflexioStruct>
class ReflexioIterator {
  using iterator_category = std::forward_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = member_descriptor_t<ReflexioStruct> const;
  using pointer = value_type*;  // or also value_type*
  using reference = value_type&;// or also value_type&
  using const_reference = value_type const&;// or also value_type&

  static constexpr ReflexioStruct::Mask no_exclusion_mask = {};
  ReflexioStruct::Mask const& m_excludeMask;
  size_t m_idx;

public:
  constexpr ReflexioIterator(size_t idx = 0,
                             ReflexioStruct::Mask const& excludeMask = no_exclusion_mask)
      : m_excludeMask(excludeMask)
      , m_idx(calc_next_index(idx)) {
  }

  [[nodiscard]] constexpr size_t calc_next_index(size_t idx) const {
    while (idx < m_excludeMask.size() and bool(m_excludeMask[idx])) {
      idx++;
    }
    return idx;
  }

  constexpr const_reference operator*() const {
    return *ReflexioStruct::get_member_descriptors()[m_idx];
  }
  constexpr pointer operator->() const {
    return ReflexioStruct::get_member_descriptors()[m_idx];
  }

  constexpr ReflexioIterator& operator++() {
    m_idx = calc_next_index(m_idx + 1);
    return *this;
  }

  constexpr ReflexioIterator operator++(int) {
    ReflexioIterator tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr friend bool operator<(ReflexioIterator const& a, ReflexioIterator const& b) { return a.m_idx < b.m_idx; };
  constexpr friend bool operator==(ReflexioIterator const& a, ReflexioIterator const& b) { return a.m_idx == b.m_idx; };
  constexpr friend bool operator!=(ReflexioIterator const& a, ReflexioIterator const& b) { return a.m_idx != b.m_idx; };
};

}// namespace reflexio
