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

#define DO_PYBIND_WRAPPING
#include <app_utils/pybind_utils.hpp>
#include <app_utils/reflexio.hpp>
#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>


ENUMATIC_DEFINE(MyEnum, 
	EnumVal1 = 1, 
	EnumVal2 , 
    EnumVal3 [[deprecated]] , 
    EnumVal4 [[deprecated]] = 5 );

ENUMATIC_DEFINE(
  MyOtherEnum, 
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

#if defined(_MSC_VER) && _MSC_VER >= 1929
#define CONSTEXPR_STRING_AND_VECTOR
#endif

#ifdef CONSTEXPR_STRING_AND_VECTOR
REFLEXIO_STRUCT_DEFINE(MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherEnum, var4, MyOtherEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var5 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var7, "var2_val", "var6 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::vector<float>, var8, {}, "var7 doc");
  );
#else
REFLEXIO_STRUCT_DEFINE(
  MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherEnum, var4, MyOtherEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var5 doc");
  );
#endif

namespace py = pybind11;

namespace app_utils::tests {
  
PYBIND11_MODULE(REFLEXIO_STRUCT_USE_PYBIND_MODULE, m) {
  // Optional docstring
  m.doc() = "app utils tests module";

#ifdef CONSTEXPR_STRING_AND_VECTOR
  m.attr("CONSTEXPR_STRING_AND_VECTOR") = py::bool_(true);
#endif
  py::register_exception<app_utils::Exception>(m, "PyException");

  py::bind_vector<std::vector<int>>(m, "VectorInt");
  py::bind_vector<std::vector<float>>(m, "VectorFloat");
  py::bind_vector<std::vector<double>>(m, "VectorDouble");
  py::bind_vector<std::vector<std::string>>(m, "VectorString");

  app_utils::pybind_utils::pybind_wrap_customizer<MyEnum>::wrap_with_pybind(m);

  app_utils::pybind_utils::pybind_wrap_customizer<MyStruct>::wrap_with_pybind(m);
}
}  // namespace app_utils::tests