#pragma once

#include <vector>
#include <span>
#include <app_utils/cond_check.hpp>
#include <cstddef>

/**
 * a resizable circular buffer based on std::vector
 */

template<typename T>
class circular_vector_t {

  size_t _capacity;
  size_t _front_index = 0;
  std::vector<T> _buffer;

public:

  class Iterator {

    circular_vector_t const* m_circ_buffer = nullptr;

  public:
    size_t m_idx = 0;

    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = T const;
    using pointer = T const*;        // or also value_type*
    using reference = T&;            // or also value_type&
    using const_reference = T const&;// or also value_type&

    Iterator() = default; // past the end operator required by LegacyForwardIterator concept
    Iterator(circular_vector_t const& _buffer, size_t index)
        : m_circ_buffer(&_buffer)
        , m_idx(index){}

    constexpr const_reference operator*() const {
      return m_circ_buffer->at(m_idx);
    }
    constexpr pointer operator->() const {
      return &(*this);
    }

    constexpr Iterator& operator++() {
      checkCond(m_circ_buffer != nullptr, "out of range access");
      m_idx++;
      if (m_idx == m_circ_buffer->_front_index) {
        m_idx = m_circ_buffer->capacity(); // end()
      } else if (m_idx == m_circ_buffer->capacity() and m_circ_buffer->_front_index != 0) {
        m_idx = 0; // wrap around
      }
      checkCond(m_idx <= m_circ_buffer->size(), "out of range access");
      return *this;
    }

    constexpr Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }

    constexpr friend bool operator==(Iterator const& a, Iterator const& b) {
      checkCond(a.m_circ_buffer == b.m_circ_buffer, "inconsistent iterators");
      return a.m_idx == b.m_idx or (a.m_idx >= a.m_circ_buffer->size() and
                                    b.m_idx >= b.m_circ_buffer->size());
    }

    constexpr friend bool operator!=(Iterator const& a, Iterator const& b) {
      return not (a == b);
    }
  };

public:
  using value_type = std::vector<T>::value_type;
  using size_type = std::vector<T>::size_type;
  using difference_type = std::vector<T>::difference_type;
  using iterator = Iterator;
  using const_iterator = Iterator;

  circular_vector_t(size_t capacity=0) : _capacity(capacity){
    _buffer.reserve(capacity);
  }

  bool operator==(circular_vector_t const& other) const {
    return  _front_index == other._front_index and _buffer == other._buffer;
  }

  [[nodiscard]]
  size_t get_front_index() const {
    return _front_index;
  }
  [[nodiscard]]
  size_t get_back_index() const {
    return (_front_index == 0 ? (_buffer.size() > 0 ? _buffer.size() - 1 : 0)
                              : _front_index - 1);
  }
  [[nodiscard]]
  size_t unwrapped_index(size_t i) const {
    if (i >= _capacity) {
      return i; // out of bound - e.g. end() iterator
    } else if (i >= _front_index) {
      return i - _front_index;
    } else {
      return _capacity - _front_index + i;
    }
  }
  [[nodiscard]]
  std::span<T const> get_values() const {
    return _buffer;
  }

  [[nodiscard]]
  T const* data() const {
    return _buffer.data();
  }

  [[nodiscard]]
  bool has_wrapped_around() const {
    return _front_index != 0;
  }

  [[nodiscard]]
  std::vector<T> const& as_vector() const {
    checkCond(not has_wrapped_around(), "circular vector cannot be interpreted as a vector once it started wrapping around");
    return _buffer;
  }

  [[nodiscard]]
  size_t size() const {
    return _buffer.size();
  }
  [[nodiscard]]
  size_t capacity() const {
    return _capacity;
  }
  [[nodiscard]]
  bool empty() const {
    return _buffer.empty();
  }

  void clear() {
    _front_index = 0;
    _buffer.clear();
  }

  void reset(size_t capacity) {
    clear();
    reserve(capacity);
  }

  void reserve(size_t capacity) {
    _capacity = capacity;
    _buffer.reserve(capacity);
  }

  T const& front() const {
    return _buffer[_front_index];
  }

  T const& back() const {
    return _buffer[get_back_index()];
  }

  T& front() {
    return _buffer[_front_index];
  }

  T& back() {
    return _buffer[get_back_index()];
  }

  Iterator begin() {
    return empty() ? end() : Iterator{*this, _front_index};
  }

  Iterator end() {
    return {*this, _capacity};
  }

  Iterator begin() const {
    return empty() ? end() : Iterator{*this, _front_index};
  }

  Iterator end() const {
    return {*this, _capacity};
  }

  T const& at(size_t index) const {
    return _buffer.at((_front_index + index) % _capacity);
  }

  size_t distance_from(size_t index) const {
    auto back_index = get_back_index();
    return back_index >= index ? back_index - index : _capacity - index + back_index;
  }

  T const& operator[](size_t index) const {
    return _buffer[(_front_index + index) % _capacity];
  }

  T& operator[](size_t index) {
    return _buffer[(_front_index + index) % _capacity];
  }

  template<typename U>
  void push_back(U&& value){
    if (_buffer.size() < _capacity){
      _buffer.push_back(std::forward<U>(value));
    } else {
      if (_front_index == _capacity - 1) {
        _front_index = 0;
      } else {
        _front_index++;
      }
      size_t index = get_back_index();
      _buffer[index] = std::forward<U>(value);
    }
  }

  template<typename ...Args>
  T& emplace_back(Args&& ... args){
    if (_buffer.size() < _capacity){
      return _buffer.emplace_back(std::forward<Args>(args)...);
    } else {
      if (_front_index == _capacity - 1) {
        _front_index = 0;
      } else {
        _front_index++;
      }
      size_t index = get_back_index();
      return _buffer[index] = T{std::forward<Args>(args)...};
    }
  }
};
