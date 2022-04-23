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

#include <app_utils/pybind_utils.hpp>
#include <app_utils/enumatic.pybind.hpp>
#include <app_utils/reflexio.pybind.hpp>
#include <app_utils/serial_utils.hpp>
#include <app_utils/cond_check.hpp>

#include "../tests/reflexio.t.hpp"

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

template <typename Tag>
struct CustomFloat {
  using underlying_type = float;

  float m_arg;
  constexpr CustomFloat(float arg = 0.) : m_arg(arg) {}

  // operator float() { return m_arg; }

  constexpr bool operator==(CustomFloat const& other) const { return m_arg == other.m_arg; }
  constexpr bool operator!=(CustomFloat const& other) const { return m_arg != other.m_arg; }

  CustomFloat operator/(float f) const { return m_arg / f; }
  CustomFloat operator*(float f) const { return m_arg * f; }
  CustomFloat& operator/=(float f) { m_arg /= f;  return *this;}
  CustomFloat& operator*=(float f) { m_arg *= f;  return *this;}

  CustomFloat operator+(CustomFloat const& f) const { return m_arg + f.m_arg; }
  CustomFloat operator-(CustomFloat const& f) const { return m_arg - f.m_arg; }
  CustomFloat& operator+=(CustomFloat const& f) { m_arg += f.m_arg; return *this; }
  CustomFloat& operator-=(CustomFloat const& f) { m_arg -= f.m_arg; return *this; }

  CustomFloat operator-() const { return -m_arg; }

  friend std::string to_string(CustomFloat const& f) {
    std::ostringstream oss;
    oss << f.m_arg << " " << app_utils::typeName<Tag>();
    return oss.str();
  }

  friend constexpr size_t serial_size(CustomFloat const& val) { 
    return app_utils::serial::serial_size(val.m_arg); 
  }

  friend constexpr size_t from_bytes(std::byte const* buffer, size_t buffer_size, CustomFloat& val) {
    return app_utils::serial::from_bytes(buffer, buffer_size, val.m_arg);
  }

  friend constexpr size_t to_bytes(std::byte* buffer, size_t buffer_size, CustomFloat const& val) {
    return app_utils::serial::to_bytes(buffer, buffer_size, val.m_arg);
  }
 };


#if defined(_MSC_VER) && _MSC_VER >= 1929
#define CONSTEXPR_STRING_AND_VECTOR
#endif

struct bla {};
struct foo {};

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

namespace app_utils::pybind {

template <>
struct pybind_wrapper_traits<CustomFloat<bla>, int> {
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
    }
  }
};

}  // namespace app_utils::pybind
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
  // note that MyOtherEnum is implicitly registered through MyStruct as it's the type of a member variable.
  pybind_wrapper<MyEnum>::wrap_with_pybind(m);
  pybind_wrapper<MyStruct>::wrap_with_pybind(m);
  pybind_wrapper<NestedStruct>::wrap_with_pybind(m);
  pybind_wrapper<SimpleStruct>::wrap_with_pybind(m);

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