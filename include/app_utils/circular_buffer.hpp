#pragma once

#include <array>


namespace app_utils {

template <typename T, size_t capacity_>
class circular_buffer_t {
  std::array<T, capacity_> m_array;
  size_t m_index = 0;

 public:
  static constexpr size_t capacity() { return capacity_; }
  constexpr bool empty() const { return m_index == 0; }

  constexpr size_t size() const { return m_index < capacity_ ? m_index : capacity_; }

  void push_back(T val) {
    m_array[m_index % capacity_] = std::move(val);
    ++m_index;
    if (m_index >= 2 * capacity_) {
      m_index = capacity_;
    }
  }

  class iterator {
   public:
    constexpr iterator(circular_buffer_t const& buffer, size_t state) 
      : m_buffer(buffer)
      , m_state(state) {}

    constexpr iterator operator++() {
      ++m_state;      
      return *this;
    }
    constexpr bool operator!=(iterator const& other) const { return m_state != other.m_state; }
    constexpr T const& operator*() const { return m_buffer.m_array[m_state % capacity_]; }

   private:
    circular_buffer_t const& m_buffer;
    size_t m_state = 0;
  };

  constexpr iterator begin() const {
    return {*this, m_index >= capacity_ ? m_index : 0};
  }
  constexpr iterator end() const { 
    return {*this, m_index >= capacity_ ? m_index + size() : m_index}; }
};
}  // namespace app_utils