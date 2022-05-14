#pragma once

#include <app_utils/serial_utils.hpp>
#include <app_utils/stream_utils.hpp>

template<typename Tag>
struct CustomFloat {
  using underlying_type = float;

  float m_arg;
  constexpr CustomFloat(float arg = 0.)
      : m_arg(arg) {}

  // operator float() { return m_arg; }

  constexpr bool operator==(CustomFloat const& other) const { return m_arg == other.m_arg; }
  constexpr bool operator!=(CustomFloat const& other) const { return m_arg != other.m_arg; }

  CustomFloat operator/(float f) const { return m_arg / f; }
  CustomFloat operator*(float f) const { return m_arg * f; }
  CustomFloat& operator/=(float f) {
    m_arg /= f;
    return *this;
  }
  CustomFloat& operator*=(float f) {
    m_arg *= f;
    return *this;
  }

  CustomFloat operator+(CustomFloat const& f) const { return m_arg + f.m_arg; }
  CustomFloat operator-(CustomFloat const& f) const { return m_arg - f.m_arg; }
  CustomFloat& operator+=(CustomFloat const& f) {
    m_arg += f.m_arg;
    return *this;
  }
  CustomFloat& operator-=(CustomFloat const& f) {
    m_arg -= f.m_arg;
    return *this;
  }

  CustomFloat operator-() const { return -m_arg; }

  friend std::string to_string(CustomFloat const& f) {
    std::ostringstream oss;
    oss << f.m_arg << " " << app_utils::typeName<Tag>();
    return oss.str();
  }

  friend void from_string(CustomFloat& val, std::string_view val_str) {
    auto items = app_utils::strutils::split(' ', val_str);
    checkCond(items.size() == 2);
    checkCond(items[1] == app_utils::typeName<Tag>());
    return app_utils::strutils::from_string(val.m_arg, items[0]);
  }

  friend constexpr size_t serial_size(CustomFloat const& val) {
    return app_utils::serial::serial_size(val.m_arg);
  }

  friend constexpr size_t from_bytes(std::byte const* buffer, size_t buffer_size, CustomFloat& val) {
    return app_utils::serial::from_bytes(buffer, buffer_size, val.m_arg);
  }

  friend constexpr size_t to_bytes(std::byte* buffer, size_t buffer_size, CustomFloat const& val) {
    return app_utils::serial::to_bytes(buffer, buffer_size, val.m_arg);
  }
};
