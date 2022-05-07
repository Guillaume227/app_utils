#pragma once

#include <pybind11/pybind11.h>
#include <pybind11/cast.h>
#include <pybind11/operators.h>
#include <pybind11/numpy.h>

#include <app_utils/pybind_utils.hpp>
#include "CustomFloat.hpp"

namespace app_utils::pybind {

template<typename Tag>
struct pybind_wrapper_traits<CustomFloat<Tag>, int> {
  constexpr static inline pybind11::return_value_policy def_readwrite_rvp =
          pybind11::return_value_policy::copy;
};

template <typename Tag>
struct pybind_wrapper<CustomFloat<Tag>, int> {

  inline static bool s_was_registered = false;

  template <class PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {
    if (not s_was_registered) {
      s_was_registered = true;
      static std::string const class_name = app_utils::typeName<CustomFloat<Tag>>();
      py::class_<CustomFloat<Tag>>(pybindHost, class_name.c_str())
              .def(py::init<>())
              .def(py::init<float>())
              .def("__copy__", [](CustomFloat<Tag> const& self_) { return CustomFloat<Tag>(self_); })
              .def("__deepcopy__", [](CustomFloat<Tag> const& self_) { return CustomFloat<Tag>(self_); })
              .def("__str__", [](CustomFloat<Tag> const& self_) { return to_string(self_); })
              //.def("__repr__", [](CustomFloat<Tag> const& self_) { return to_string(self_); })
              .def(py::self == py::self)
              .def(py::self != py::self)
              .def(py::self *= float())
              .def(py::self /= float())
              .def(py::self * float())
              .def(py::self / float())
              .def(py::self + py::self)
              .def(py::self += py::self)
              .def(py::self - py::self)
              .def(py::self -= py::self)
              .def(-py::self);

      py::implicitly_convertible<float, CustomFloat<Tag>>();

      // numpy compatibility

    }
  }
};
}  // namespace app_utils::pybind

namespace pybind11::detail {
template<typename Tag>
struct npy_format_descriptor<CustomFloat<Tag>, std::enable_if_t<true>> {
private:
  using base_descr = npy_format_descriptor<typename CustomFloat<Tag>::underlying_type>;

public:
  static constexpr auto name = base_descr::name;

  static std::string format() {
    return ::pybind11::format_descriptor<typename CustomFloat<Tag>::underlying_type>::format();
  }

  static pybind11::dtype dtype() { return base_descr::dtype(); }
};
}// namespace pybind11::detail
