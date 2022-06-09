#ifdef WIN32
struct timeval;  // Windows-specific: forward declaration to fix compilation error in pytime.h: error C2335: 'timeval':
                 // a type cannot be introduced in a function parameter list
#define strdup _strdup  // Windows-specific: https://github.com/pybind/pybind11/issues/1212
#endif

#include "CustomFloat.pybind.hpp"
#include <app_utils/enumatic.pybind.hpp>
#include <app_utils/reflexio.pybind.hpp>
#include <app_utils/serial_utils.hpp>
#include <app_utils/cond_check.hpp>
#include <app_utils/async.pybind.hpp>
#include "../tests/reflexio.t.hpp"


PYBIND11_MAKE_OPAQUE(ArrayFloat8_t);
PYBIND11_MAKE_OPAQUE(std::vector<int>);
PYBIND11_MAKE_OPAQUE(std::vector<float>);
PYBIND11_MAKE_OPAQUE(std::vector<double>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string>);
PYBIND11_MAKE_OPAQUE(std::vector<std::string_view>);

#if defined(_MSC_VER) && _MSC_VER >= 1929
#define CONSTEXPR_STRING_AND_VECTOR
#endif

// test different return value policies for different CustomFloat tags
namespace app_utils::pybind {
template<>
struct pybind_wrapper_traits<CustomFloat<foo>, int> {
  constexpr static inline pybind11::return_value_policy def_readwrite_rvp =
          pybind11::return_value_policy::reference;
};
} // namespace app_utils::pybind

#ifdef CONSTEXPR_STRING_AND_VECTOR
REFLEXIO_STRUCT_DEFINE(MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherEnum, var4, MyOtherEnum::EnumVal2, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var5 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var6 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<bla>, var7, 22.2f, "var7 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<foo>, var8, 11.1f, "var8 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var_string, "var_string_val", "var string doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::vector<float>, var_vect, {}, "var vect doc");
  );
#else
REFLEXIO_STRUCT_DEFINE(
  MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<bla>, var2, 1.5f, "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherEnum, var4, MyOtherEnum::EnumVal2, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var5 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var6 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<bla>, var7, 22.2f, "var7 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<foo>, var8, 11.1f, "var8 doc");
  );
#endif


REFLEXIO_STRUCT_DEFINE(
  SimpleStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
);

static_assert(std::is_standard_layout<SimpleStruct>());
static_assert(std::is_trivially_copyable<SimpleStruct>());

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
  py::bind_vector<std::vector<std::string_view>>(m, "VectorStringView");

  using app_utils::pybind::pybind_wrapper;
  // explicit registration of MyEnum on the module.
  // note that MyOtherEnum is implicitly registered through MyStruct as it's the type of a member variable  .
  pybind_wrapper<MyEnum>::wrap_with_pybind(m);
  pybind_wrapper<MyStruct>::wrap_with_pybind(m);
  pybind_wrapper<NestedStruct>::wrap_with_pybind(m);
  pybind_wrapper<SimpleStruct>::wrap_with_pybind(m);


  pybind_wrapper<std::future<SimpleStruct>>::wrap_with_pybind(m);

  m.def("make_simple_struct_future", []() {
    return std::async([] {
      return SimpleStruct{};
    });
  });

  m.def("get_simple_struct_array", [](size_t n){
    auto arr = app_utils::pybind::mkarray_via_buffer<SimpleStruct>(n);

    auto req = arr.request();
    auto *ptr = static_cast<SimpleStruct *>(req.ptr);
    for (size_t i = 0; i < n; i++) {
      auto& struct_ptr = ptr[i];
      struct_ptr.var1 = (int)i;
      struct_ptr.var3 = static_cast<MyEnum>(i % Enumatic<MyEnum>::size());
    }
    return arr;
  });
}
}  // namespace app_utils::tests