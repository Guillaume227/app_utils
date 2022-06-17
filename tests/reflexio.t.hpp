#pragma once

#include <app_utils/reflexio.hpp>

#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>
#include "CustomFloat.hpp"

#if defined(_MSC_VER) && _MSC_VER >= 1929
//#define CONSTEXPR_STRING_AND_VECTOR
#endif

ENUMATIC_DEFINE(TestEnum, EnumVal1, EnumVal2);

static_assert(sizeof(TestEnum) == 4);
static_assert(serial_size(TestEnum{}) == 1);

REFLEXIO_STRUCT_DEFINE(SingleVarStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 14, "var1 doc");
  static_assert(member_var_counter_t<__var1_id>::index == 0);
  static_assert(member_var_traits_t<0, int>::descriptor != nullptr););

static_assert(SingleVarStruct().var1 < 15);

// static_assert(serial_size(SingleVarStruct{}) == 4); // seems to work on G++, fails currently on windows (complain pointer
// to object conversion from void* is not a constant expression)
//static_assert(SingleVarStruct().has_all_default_values());

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, .5f, "var2 doc"););


REFLEXIO_STRUCT_DEFINE(TrivialStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int8_t, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.1f, "var2 doc"););

static_assert(std::is_standard_layout_v<TrivialStruct>);
static_assert(std::is_trivially_copyable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_assignable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_constructible_v<TrivialStruct>);
//static_assert(std::is_trivially_constructible_v<TrivialStruct>); // NO: because of in-line initialization

REFLEXIO_STRUCT_DEFINE(NestedStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(MyOtherStruct, struct1, {}, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(TrivialStruct, struct2, {}, "var2 doc"););

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

struct bla {}; // inherits default CustomFlag copy policy
struct foo {}; // overrides default below

using ArrayFloat8_t = std::array<float, 8>;
REFLEXIO_STRUCT_DEFINE(
    FancierStruct,
    REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var2 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(MyEnum, var3, MyEnum::EnumVal2, "var3 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(MyOtherEnum, var4, MyOtherEnum::EnumVal2, "var4 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(bool, var5, true, "var5 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(ArrayFloat8_t, var6, {0}, "var6 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<bla>, var7, 22.2f, "var7 doc");
    REFLEXIO_MEMBER_VAR_DEFINE(CustomFloat<foo>, var8, 11.1f, "var8 doc");
);