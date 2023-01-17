#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/functional.h>
#include <pybind11/operators.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/numpy.h>

#include <app_utils/stream_utils.hpp>
#include <app_utils/string_utils.hpp>

namespace app_utils::pybind {

namespace py = pybind11;

template <typename T, typename Dummy = int>
struct pybind_wrapper_traits {
  constexpr static inline py::return_value_policy def_readwrite_rvp =
      py::return_value_policy::reference_internal; // that value is pybind default
};

template <typename T, typename Dummy = int>
struct pybind_wrapper {

  constexpr static inline auto return_value_policy = py::return_value_policy::automatic;

  template <class PybindHost>
  static void wrap_with_pybind(PybindHost&) requires std::is_arithmetic_v<T> or 
                                                     std::is_same_v<T, std::string> or
                                                     std::is_same_v<T, std::string_view> or
                                                     std::is_same_v<T, std::complex<float>> or
                                                     std::is_same_v<T, std::complex<double>>{}
};

template <typename T>
struct pybind_wrapper<std::vector<T>> {  

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost&) {
    // Do nothing, vectors are wrapped separately from py primitives
  }
};

template <typename T, size_t N>
struct pybind_wrapper<std::array<T, N>> {
  
  using ArrayType = std::array<T, N>;

  inline static bool s_registered_once = false;

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost& m) {  

    using SizeType = ArrayType::size_type;
    using DiffType = ArrayType::difference_type;

    if(not s_registered_once) {
      s_registered_once = true;
      auto wrap_i = [](DiffType i, SizeType n) -> SizeType {
        if (i < 0) i += n;
        if (i < 0 || (SizeType)i >= n) throw py::index_error();
        return i;
      };

      static std::string_view const typeName = app_utils::typeName<ArrayType>();
      auto cl = py::class_<ArrayType>(m, typeName.data());
      cl.def(py::init<>())
          .def(py::self == py::self)
          .def(py::self != py::self)
          .def("__str__", [](ArrayType const& self_) { return app_utils::strutils::to_string(self_); })
          .def("__len__", &ArrayType::size)
          .def("__setitem__",
               [wrap_i](ArrayType& v, DiffType i, const T& t) {
                 SizeType index = wrap_i(i, v.size());
                 v[index] = t;
               })
          .def("__getitem__", [wrap_i](ArrayType& v, DiffType i) {
            SizeType index = wrap_i(i, v.size());
            return v[index];
          });
    }
  }
};


template <typename T>
py::array mkarray_via_buffer(size_t n) {
  return py::array(py::buffer_info(
          nullptr, sizeof(T), py::format_descriptor<T>::format(), 1, {n}, {sizeof(T)}));
}

}  // namespace app_utils::pybind