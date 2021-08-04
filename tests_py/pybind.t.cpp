#ifdef WIN32
struct timeval;  // Windows-specific: forward declaration to fix compilation error in pytime.h: error C2335: 'timeval':
                 // a type cannot be introduced in a function parameter list
#define strdup _strdup  // Windows-specific: https://github.com/pybind/pybind11/issues/1212
#endif

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>
#include <pybind11/functional.h>


#include <app_utils/relfexio.hpp>
#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

ENUMATIC_DEFINE(TestEnum, EnumVal1, EnumVal2);

REFLEXIO_STRUCT_DEFINE(MyStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var3, 1.5f, "var3 doc"););


namespace py = pybind11;

namespace app_utils::tests {
  
PYBIND11_MODULE(REFLEXIO_STRUCT_USE_PYBIND_MODULE, m) {
  // Optional docstring
  m.doc() = "app utils tests module";

  py::register_exception<app_utils::Exception>(m, "PyException");

  py::wrap_reflexio_struct<MyStruct>(m);

  py::wrap_enumatic<TestEnum>(m);
}
}  // namespace app_utils::tests