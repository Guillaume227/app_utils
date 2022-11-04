#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/operators.h>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>

#include <app_utils/circular_vector.hpp>
#include <app_utils/stream_utils.hpp>
#include <app_utils/string_utils.hpp>

namespace app_utils::pybind {

namespace py = pybind11;

template <typename T>
struct pybind_wrapper<circular_vector_t<T>> {
  
  using WrappedType = circular_vector_t<T>;

  inline static bool s_registered_once = false;

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost& m) {  

    using SizeType = WrappedType::size_type;
    using DiffType = WrappedType::difference_type;

    if(not s_registered_once) {

      s_registered_once = true;

      auto wrap_i = [](DiffType i, SizeType n) -> SizeType {
        if (i < 0) i += n;
        if (i < 0 || (SizeType)i >= n) throw py::index_error();
        return i;
      };

      static std::string_view const typeName = app_utils::typeName<WrappedType>();
      auto cl = py::class_<WrappedType>(m, typeName.data());
      cl.def(py::init<>())
        .def(py::self == py::self)
        .def(py::self != py::self)
        .def("__str__", [](WrappedType const& self_) {
          return app_utils::strutils::range_to_string(self_);
        })
        .def("__len__", &WrappedType::size)
        .def("__setitem__",
             [wrap_i](WrappedType& v, DiffType i, T const& t) {
               SizeType index = wrap_i(i, v.size());
               v[index] = t;
             })
        .def("__getitem__", [wrap_i](WrappedType& v, DiffType i) {
          SizeType index = wrap_i(i, v.size());
          return v[index];
        });
    }
  }
};

}  // namespace app_utils::pybind