#pragma once

#ifndef DO_PYBIND_WRAPPING
#define DO_PYBIND_WRAPPING
#endif
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "reflexio.hpp"

namespace app_utils::pybind {

namespace py = pybind11;

template <typename ReflexioStruct>
struct pybind_wrapper<ReflexioStruct,
                      std::enable_if_t<reflexio::is_reflexio_struct<ReflexioStruct>::value, int>> {
  inline static bool s_registered_once = false;

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {

    if (not s_registered_once) {
      s_registered_once = true;
    
      static std::string const typeName = app_utils::typeName<ReflexioStruct>();
      auto wrappedType = typename ReflexioStruct::PybindClassType(pybindHost, typeName.c_str());
      wrappedType.def(pybind11::init<>())
          .def_property_readonly_static("__doc__", [](py::object /*self*/){
            return ReflexioStruct::get_docstring();
          })
          .def("__deepcopy__", [](ReflexioStruct const& self, py::dict) { return ReflexioStruct(self); })
          .def(pybind11::self == pybind11::self)
          .def(pybind11::self != pybind11::self)
          .def("__str__", [](ReflexioStruct const& self_) { return to_string(self_); })
          .def("as_dict",
               [](ReflexioStruct const& self_) {
                 py::dict dico;
                 for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {                   
                   dico[member_descriptor->get_name().data()] = member_descriptor->get_py_value(self_);
                 }
                 return dico;
               })
          .def("__len__", [](ReflexioStruct const&) { return ReflexioStruct::get_member_descriptors().size(); })
          //.def("__iter__",
          //     [](ReflexioStruct const& self_) { return py::make_iterator(self_.begin(), self_.end()); },
          //     py::keep_alive<0, 1>())
          .def("__getitem__", [](ReflexioStruct const& self_, std::string_view name) { 
                  for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {
                   if (name == member_descriptor->get_name()) {
                     return member_descriptor->get_py_value(self_);
                   }
                 }
                 throw py::key_error("key '" + std::string{name} + "' does not exist");                    
          })
          .def("__setitem__",
               [](ReflexioStruct& self_, std::string_view name, py::object const& value) {
                 for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {
                   if (name == member_descriptor->get_name()) {
                     member_descriptor->set_py_value(self_, value);
                     return;
                   }
                 }
                 throw py::key_error("key '" + std::string{name} + "' does not exist");
               })
          .def("get_serial_size", static_cast<size_t(ReflexioStruct::*)() const>(&ReflexioStruct::get_serial_size))
          .def("deserialize",
               [&](ReflexioStruct& self, pybind11::bytes const& data) {
                 std::string dataStr(data);
                 return from_bytes((std::byte const*)dataStr.c_str(), (unsigned int)dataStr.size(), self);
               })
          .def("serialize",
               [&](ReflexioStruct const& self) {
                 std::string dataStr(serial_size(self), '\0');
                 to_bytes((std::byte*)dataStr.c_str(), (unsigned int)dataStr.size(), self);
                 return pybind11::bytes(dataStr);
               })
          .def("non_default_values", &ReflexioStruct::non_default_values)
          .def("has_all_default_values", &ReflexioStruct::has_all_default_values)
          .def("differing_members", [](ReflexioStruct const& _self, ReflexioStruct const& other){ return _self.differing_members(other);})
          .def("differences", [](ReflexioStruct const& _self, ReflexioStruct const& other){ return _self.differences(other);});

      for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {
        member_descriptor->wrap_with_pybind(pybindHost, &wrappedType);
      }

      // Numpy compatibility

      if constexpr(std::is_standard_layout<ReflexioStruct>()) {
        std::vector<::pybind11::detail::field_descriptor> field_descriptors;
        for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {
          bool added = member_descriptor->add_numpy_descriptor(field_descriptors);
          checkCond(added, "should never hit that line unless all member variables can be registered");
        }
        ::pybind11::detail::npy_format_descriptor<ReflexioStruct>::register_dtype(field_descriptors);
        wrappedType.def_property_readonly_static("dtype",
                               [](py::object /*self*/){ return ::pybind11::detail::npy_format_descriptor<ReflexioStruct>::dtype(); });
      }
    }
  }
};
}  // namespace app_utils::pybind
