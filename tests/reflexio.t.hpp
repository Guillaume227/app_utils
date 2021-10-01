#pragma once

#include <app_utils/reflexio.hpp>

#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

#if defined(_MSC_VER) && _MSC_VER >= 1929
#define CONSTEXPR_STRING_AND_VECTOR
#endif

ENUMATIC_DEFINE(TestEnum, EnumVal1, EnumVal2);

static_assert(sizeof(TestEnum) == 4);
static_assert(serial_size(TestEnum{}) == 1);

REFLEXIO_STRUCT_DEFINE(MiniStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 14, "var1 doc");
  static_assert(member_var_counter_t<__var1_id>::index == 0);
  static_assert(member_var_descriptor_t<0, int>::descriptor != nullptr););

static_assert(MiniStruct().var1 < 15);

// static_assert(serial_size(MiniStruct{}) == 4); // seems to work on G++, fails currently on windows (complain pointer
// to object conversion from void* is not a constant expression)
//static_assert(MiniStruct().has_all_default_values());

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, .5f, "var2 doc"););


REFLEXIO_STRUCT_DEFINE(TrivialStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.1f, "var2 doc"););

static_assert(std::is_standard_layout_v<TrivialStruct>);
static_assert(std::is_trivially_copyable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_assignable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_constructible_v<TrivialStruct>);
//static_assert(std::is_trivially_constructible_v<TrivialStruct>); // NO: because of in-line initialization

REFLEXIO_STRUCT_DEFINE(NestedStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherStruct, var1, {}, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(TrivialStruct, var2, {}, "var2 doc"););
