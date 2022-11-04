#pragma once

#include <array>
#include <cstddef>

namespace app_utils {

template <typename T, size_t capacity_>
class circular_array_t {
  std::array<T, capacity_> m_array;
  size_t m_index = 0;

 public:
  static constexpr size_t capacity() { return capacity_; }
  constexpr bool empty() const { return m_index == 0; }

  constexpr size_t size() const { return m_index < capacity_ ? m_index : capacity_; }

  constexpr T& get_next_slot() {
    auto& next_slot = m_array[m_index % capacity_];
    ++m_index;
    if (m_index >= 2 * capacity_) {
      m_index = capacity_;
    }
    return next_slot;
  }

  class iterator {
   public:
    constexpr iterator(circular_array_t& buffer, size_t state)
      : m_buffer(buffer)
      , m_state(state) {}

    constexpr iterator& operator++() {
      ++m_state;      
      return *this;
    }
    constexpr bool operator!=(iterator const& other) const { return m_state != other.m_state; }
    constexpr T const& operator*() const { return m_buffer.m_array[m_state % capacity_]; }
    constexpr T& operator*() { return m_buffer.m_array[m_state % capacity_]; }

   private:
    circular_array_t& m_buffer;
    size_t m_state = 0;
  };

  class const_iterator {
  public:
    constexpr const_iterator(circular_array_t const& buffer, size_t state)
            : m_buffer(buffer)
            , m_state(state) {}

    constexpr const_iterator& operator++() {
      ++m_state;
      return *this;
    }
    constexpr bool operator!=(const_iterator const& other) const { return m_state != other.m_state; }
    constexpr T const& operator*() const { return m_buffer.m_array[m_state % capacity_]; }

  private:
    circular_array_t const& m_buffer;
    size_t m_state = 0;
  };

  constexpr iterator begin() { return {*this, m_index >= capacity_ ? m_index : 0}; }
  constexpr iterator end() { return {*this, m_index >= capacity_ ? m_index + size() : m_index}; }
  constexpr const_iterator begin() const { return {*this, m_index >= capacity_ ? m_index : 0}; }
  constexpr const_iterator end() const { return {*this, m_index >= capacity_ ? m_index + size() : m_index}; }
};
}  // namespace app_utils