#ifdef WIN32
struct timeval;  // Windows-specific: forward declaration to fix compilation error in pytime.h: error C2335: 'timeval':
                 // a type cannot be introduced in a function parameter list
#define strdup _strdup  // Windows-specific: https://github.com/pybind/pybind11/issues/1212
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>
#include <pybind11/cast.h>
#include <pybind11/functional.h>
#include <pybind11/operators.h>


#include <app_utils/reflexio.hpp>
#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

namespace pybind11 {

template<typename Array>
void bind_std_array(module_& m) {

  using T = typename Array::value_type;
  using SizeType = typename Array::size_type;
  using DiffType = typename Array::difference_type;

  auto wrap_i = [](DiffType i, SizeType n) -> SizeType {
    if (i < 0) i += n;
    if (i < 0 || (SizeType)i >= n) throw index_error();
    return i;
  };

  static std::string const typeName = app_utils::typeName<Array>();
  auto cl = pybind11::class_<Array>(m, typeName.c_str());
  cl.def(pybind11::init<>())
      .def(pybind11::self == pybind11::self)
      .def(pybind11::self != pybind11::self)
      .def("__str__", [](Array const& self_) { return app_utils::strutils::to_string(self_); })
      .def("__len__", &Array::size)
      .def("__setitem__", [wrap_i](Array& v, DiffType i, const T& t) {
                            SizeType index = wrap_i(i, v.size());
                            v[index] = t;
                            }
          )
      .def("__getitem__", [wrap_i](Array& v, DiffType i) {
        SizeType index = wrap_i(i, v.size());
        return v[index];
      });

}
}  // namespace pybind11

ENUMATIC_DEFINE(MyEnum, 
	EnumVal1, 
	EnumVal2, 
    EnumVal3);

using ArrayFloat8_t = std::array<float, 8>;
PYBIND11_MAKE_OPAQUE(ArrayFloat8_t);
PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<float>);
PYBIND11_MAKE_OPAQUE(std::vector<double>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string_view>);


REFLEXIO_STRUCT_DEFINE(MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var3, 1.5f, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var4, MyEnum::EnumVal2, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var5 doc");  
  REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var6 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::vector<float>, var7, {}, "var7 doc");
  );

namespace py = pybind11;

namespace app_utils::tests {
  
PYBIND11_MODULE(REFLEXIO_STRUCT_USE_PYBIND_MODULE, m) {
  // Optional docstring
  m.doc() = "app utils tests module";

  py::register_exception<app_utils::Exception>(m, "PyException");

  py::bind_vector<std::vector<int>>(m, "VectorInt");
  py::bind_vector<std::vector<float>>(m, "VectorFloat");
  py::bind_vector<std::vector<double>>(m, "VectorDouble");
  py::bind_vector<std::vector<std::string>>(m, "VectorString");

  py::bind_std_array<ArrayFloat8_t>(m);
  py::wrap_reflexio_struct<MyStruct>(m);

  py::wrap_enumatic<MyEnum>(m);
}
}  // namespace app_utils::tests