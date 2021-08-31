#pragma once

#include <pybind11/cast.h>
#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <app_utils/stream_utils.hpp>
#include <app_utils/string_utils.hpp>

namespace app_utils::pybind_utils {

template <typename T, typename Dummy=int>
struct pybind_wrap_customizer {
  template <class PybindHost>
  static void wrap_with_pybind(PybindHost&) {}
};

template <typename Array, typename PybindHost>
void bind_std_array(PybindHost& m) {
  using T = typename Array::value_type;
  using SizeType = typename Array::size_type;
  using DiffType = typename Array::difference_type;

  auto wrap_i = [](DiffType i, SizeType n) -> SizeType {
    if (i < 0) i += n;
    if (i < 0 || (SizeType)i >= n) throw pybind11::index_error();
    return i;
  };

  static std::string const typeName = app_utils::typeName<Array>();
  auto cl = pybind11::class_<Array>(m, typeName.c_str());
  cl.def(pybind11::init<>())
      .def(pybind11::self == pybind11::self)
      .def(pybind11::self != pybind11::self)
      .def("__str__", [](Array const& self_) { return app_utils::strutils::to_string(self_); })
      .def("__len__", &Array::size)
      .def("__setitem__",
           [wrap_i](Array& v, DiffType i, const T& t) {
             SizeType index = wrap_i(i, v.size());
             v[index] = t;
           })
      .def("__getitem__", [wrap_i](Array& v, DiffType i) {
        SizeType index = wrap_i(i, v.size());
        return v[index];
      });
}
}
