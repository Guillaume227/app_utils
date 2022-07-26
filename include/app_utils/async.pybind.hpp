#pragma once

#include <pybind11/stl.h>
#include <pybind11/pybind11.h>
#include <future>
#include <memory>

namespace app_utils::pybind {

template<typename ResultType>
struct pybind_wrapper<std::future<ResultType>, int> {

  template<typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {
    static std::string_view const typeName = app_utils::typeName<std::future<ResultType>>();
    auto wrappedType = pybind11::class_<std::future<ResultType>>(pybindHost, typeName.data());
    wrappedType.def("get", &std::future<ResultType>::get);
  }
};
/*
template<typename ResultType>
class stop_iteration : public pybind11::builtin_exception {
  std::shared_ptr<ResultType> _value;
public:
  using builtin_exception::builtin_exception;

  stop_iteration(std::shared_ptr<ResultType> value = nullptr)
          : stop_iteration("") {_value = value;}

  void set_error() const override { PyErr_SetString(PyExc_StopIteration, what()); }

  std::shared_ptr<ResultType> value() {
    return _value;
  }
};

template<typename ResultType>
class Awaitable : public std::enable_shared_from_this<Awaitable<ResultType>> {
public:
  Awaitable() {}
  Awaitable(std::future<ResultType> future)
          : _future(std::move(future)) {}

  std::shared_ptr<Awaitable<ResultType>> __iter__() {
    return this->shared_from_this();
  }

  std::shared_ptr<Awaitable<ResultType>> __await__() {
    return this->shared_from_this();
  }

  void __next__() {
    if (_future.wait_for(std::chrono::seconds{0}) == std::future_status::ready) {
      auto value = std::make_shared<ResultType>(_future.get());
      throw app_utils::pybind::stop_iteration<ResultType>(std::move(value));
    }
  }

private:
  std::future<ResultType> _future;
};


template<typename ResultType>
struct pybind_wrapper<Awaitable<ResultType>, int> {

  template<typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {

    static std::string_view const typeName = app_utils::typeName<Awaitable<ResultType>>();
    auto wrappedType = pybind11::class_<Awaitable<ResultType>, std::shared_ptr<Awaitable<ResultType>>>(pybindHost,
                                                                                                       typeName.c_str());
    wrappedType.def(pybind11::init<>())
            .def("__await__", &Awaitable<ResultType>::__await__)
            .def("__iter__", &Awaitable<ResultType>::__iter__)
            .def("__next__", &Awaitable<ResultType>::__next__);
  }
};

template<typename ResultType>
struct pybind_wrapper<stop_iteration<ResultType>, int> {

  template<typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {
    static std::string_view const typeName = app_utils::typeName<stop_iteration<ResultType>>();
    auto wrappedType = pybind11::class_<stop_iteration<ResultType>>(pybindHost, typeName.c_str());
    wrappedType.def_property_readonly("value", &stop_iteration<ResultType>::value);
  }
};
*/
}  // namespace app_utils::pybind
/*
namespace pybind11 {
namespace detail {

template<typename T>
struct type_caster<std::future<T>> {
public:
PYBIND11_TYPE_CASTER(std::future<T>, const_name(app_utils::typeName<std::future<T>>()));

  bool load(pybind11::handle src, bool convert) {
    return false;
  }

  static pybind11::handle cast(std::future<T> && src, pybind11::return_value_policy policy, pybind11::handle parent) {
    py::array a(std::move(src.shape()), std::move(src.strides()), src.data());

    return a.release();
  }
};

}
} // namespace pybind11::detail
*/
