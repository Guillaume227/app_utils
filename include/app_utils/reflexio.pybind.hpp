#pragma once

#ifndef DO_PYBIND_WRAPPING
#define DO_PYBIND_WRAPPING
#endif
#include <pybind11/numpy.h>
#include <pybind11/stl.h>
#include "reflexio.hpp"

namespace app_utils::pybind {

namespace py = pybind11;

template<typename ReflexioStruct>
void inject_view_attributes(auto& pyobject,
                            reflexio::reflexio_fat_view <ReflexioStruct> const& view,
                            pybind11::return_value_policy rvp = pybind11::return_value_policy::reference) {
  //auto ns = py::cast(view, rvp);//py::module_::import("types").attr("SimpleNamespace")();
  for (auto& member_descriptor: view) {
    py::setattr(pyobject, member_descriptor.get_name().data(), member_descriptor.get_py_value(&view.object, rvp));
  }
}
} // namespace app_utils::pybind

namespace pybind11::detail {
// inspiration there: https://github.com/pybind/pybind11/issues/1176
template <typename ReflexioStruct> struct type_caster<reflexio::reflexio_fat_view<ReflexioStruct>>
: public type_caster_base<reflexio::reflexio_fat_view<ReflexioStruct>> {
using base = type_caster_base<reflexio::reflexio_fat_view<ReflexioStruct>>;
public:
  bool load(handle src, bool convert) {
    return base::load(src, convert);
  }

  static handle cast(reflexio::reflexio_fat_view<ReflexioStruct> *src, return_value_policy policy, handle parent) {
    auto ret = base::cast(src, policy, parent);
    app_utils::pybind::inject_view_attributes(ret, *src, policy);
    return ret;
  }

  static handle cast(reflexio::reflexio_fat_view<ReflexioStruct>& src, return_value_policy policy, handle parent) {
    auto ret = base::cast(src, policy, parent);
    app_utils::pybind::inject_view_attributes(ret, src, policy);
    return ret;
  }

  static handle cast(reflexio::reflexio_fat_view<ReflexioStruct> src, return_value_policy policy, handle parent) {
    auto ret = base::cast(std::move(src), policy, parent);
    app_utils::pybind::inject_view_attributes(ret, src, policy);
    return ret;
  }
};

} // namespace pybind11::detail

namespace app_utils::pybind {

namespace py = pybind11;

template <typename ReflexioStruct>
struct pybind_wrapper<ReflexioStruct,
                      std::enable_if_t<reflexio::is_reflexio_struct<ReflexioStruct>::value, int>> {
  inline static bool s_registered_once = false;

  template <typename PybindHost>
  static void wrap_view(PybindHost& pybindHost) {

    using View = reflexio::reflexio_fat_view<ReflexioStruct>;
    static std::string const typeName = std::string{app_utils::typeName<ReflexioStruct>(/*strip namespace*/true)} + "_fview";

    // 'Base' python type (what we can wrap at compile time) for a reflexio view.
    // Actual visible fields need to be inserted at runtime, see custom type caster logic above.
    auto wrappedType = pybind11::class_<View>(pybindHost, typeName.data(), py::dynamic_attr());
    wrappedType
      .def_property_readonly_static("__doc__", [](py::object /*self*/){
        return ReflexioStruct::get_docstring();
      })
      //.def(pybind11::self == pybind11::self)
      //.def(pybind11::self != pybind11::self)
      .def("__str__", [](View const& self_) { return to_yaml(self_); })
      .def("object", [](View const& self_) { return self_.object; })
      .def("__len__", [](View const& view) { return view.exclude_mask.size() - view.exclude_mask.count(); },
           "return the number of populated (non-stale) fields")
      .def("__getitem__", [](View const& self_, std::string_view name) {
        for (auto& member_descriptor : ReflexioStruct::get_member_descriptors(self_.exclude_mask)) {
          if (name == member_descriptor.get_name()) {
            return member_descriptor.get_py_value(&self_.object, pybind11::return_value_policy::reference);
          }
        }
        throw py::key_error("key '" + std::string{name} + "' does not exist");
      })
      .def("__setitem__",
           [](View& self_, std::string_view name, py::object const& value) {
             for (auto& member_descriptor : ReflexioStruct::get_member_descriptors(self_.exclude_mask)) {
               if (name == member_descriptor.get_name()) {
                 member_descriptor.set_py_value(&self_.object, value);
                 return;
               }
             }
             throw py::key_error("key '" + std::string{name} + "' does not exist");
           })
      ;
  }

  template <typename PybindHost>
  static void wrap_with_pybind(PybindHost& pybindHost) {

    if (not s_registered_once) {
      s_registered_once = true;

      static std::string_view const typeName = app_utils::typeName<ReflexioStruct>(/*strip namespace*/true);
      auto wrappedType = typename ReflexioStruct::PybindClassType(pybindHost, typeName.data());
      wrappedType.def(pybind11::init<>())
          .def_property_readonly_static("__doc__", [](py::object /*self*/){
            return ReflexioStruct::get_docstring();
          })
          .def("__deepcopy__", [](ReflexioStruct const& self, py::dict) { return ReflexioStruct(self); })
          .def(pybind11::self == pybind11::self)
          .def(pybind11::self != pybind11::self)
          .def("__str__", [](ReflexioStruct const& self_) { return to_yaml(self_); })
          .def("as_dict",
               [](ReflexioStruct const& self_) {
                 py::dict dico;
                 for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {                   
                   dico[member_descriptor->get_name().data()] =
                           member_descriptor->get_py_value(&self_, pybind11::return_value_policy::reference);
                        // note the return_value_policy::reference here - gives non owning semantics. Should we switch to ::copy?
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
                     return member_descriptor->get_py_value(&self_, pybind11::return_value_policy::reference);
                   }
                 }
                 throw py::key_error("key '" + std::string{name} + "' does not exist");
          })
          .def("__setitem__",
               [](ReflexioStruct& self_, std::string_view name, py::object const& value) {
                 for (auto member_descriptor : ReflexioStruct::get_member_descriptors()) {
                   if (name == member_descriptor->get_name()) {
                     member_descriptor->set_py_value(&self_, value);
                     return;
                   }
                 }
                 throw py::key_error("key '" + std::string{name} + "' does not exist");
               })
         /**
          * getitem and setitem that take an index: required for numpy compatibility
          */
          .def("__getitem__", [](ReflexioStruct const& self_, size_t index) {
            if (index >= self_.num_registered_member_vars()) {
              throw py::key_error("key '" + std::to_string(index) + "' does not exist");
            }
            auto member_descriptor = ReflexioStruct::get_member_descriptors()[index];
            return member_descriptor->get_py_value(&self_, pybind11::return_value_policy::reference);
          })
          .def("__setitem__",
           [](ReflexioStruct& self_, size_t index, py::object const& value) {
              if (index >= self_.num_registered_member_vars()) {
                throw py::key_error("key '" + std::to_string(index) + "' does not exist");
              }
              auto member_descriptor = ReflexioStruct::get_member_descriptors()[index];
              return member_descriptor->set_py_value(&self_, value);
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

      wrap_view(pybindHost);
    }
  }
};

}  // namespace app_utils::pybind
