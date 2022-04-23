#include <catch2/catch.hpp>
#include <type_traits>

#include <app_utils/enumatic.hpp>
#include <app_utils/serial_utils.hpp>
#include <app_utils/serial_buffer.hpp>

ENUMATIC_DEFINE(MyEnum, valneg = -1, val1 = 1, val2 = 2, val3, val4 = -3, val5,);

static_assert(std::is_standard_layout<MyEnum>());
static_assert(std::is_enum<MyEnum>());

TEST_CASE("parse_enum", "[enumatic]") { 
  auto enum_details = enumatic::details::parse_enum_definition<4>("valneg [[deprecated]]=  -1, val1 = 1, val2 [[deprecated]], val3  , "); 
  REQUIRE(enum_details[0].value_name == "valneg");
  REQUIRE(enum_details[1].value_name == "val1");
  REQUIRE(enum_details[2].value_name == "val2");
  REQUIRE(enum_details[3].value_name == "val3");

  REQUIRE(enum_details[0].int_value == -1);
  REQUIRE(enum_details[1].int_value == 1);  
  REQUIRE(enum_details[2].int_value == 2);
  REQUIRE(enum_details[3].int_value == 3);
}

TEST_CASE("enumatic_static_cast", "[enumatic]") {
  REQUIRE(static_cast<int>(MyEnum::valneg) == -1);
  REQUIRE(static_cast<int>(MyEnum::val1) == 1);
  REQUIRE(static_cast<int>(MyEnum::val2) == 2);
  REQUIRE(static_cast<int>(MyEnum::val3) == 3);
  REQUIRE(static_cast<int>(MyEnum::val4) == -3);
  REQUIRE(static_cast<int>(MyEnum::val5) == -2);
}

TEST_CASE("enumatic::get_underlying_value", "[enumatic]") {
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::valneg) == -1);
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::val1) == 1);
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::val2) == 2);
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::val3) == 3);
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::val4) == -3);
  REQUIRE(Enumatic<MyEnum>::get_underlying_value(MyEnum::val5) == -2);
}

TEST_CASE("enumatic::get_values", "[enumatic]") {
  auto const vals_array = Enumatic<MyEnum>::get_values();

  REQUIRE(vals_array[0] == MyEnum::valneg);
  REQUIRE(vals_array[1] == MyEnum::val1);
  REQUIRE(vals_array[2] == MyEnum::val2);
  REQUIRE(vals_array[3] == MyEnum::val3);
  REQUIRE(vals_array[4] == MyEnum::val4);
  REQUIRE(vals_array[5] == MyEnum::val5);
}

TEST_CASE("enumatic::get_index", "[enumatic]") {
  REQUIRE(0 == get_index(MyEnum::valneg));
  REQUIRE(1 == get_index(MyEnum::val1));
  REQUIRE(2 == get_index(MyEnum::val2));
  REQUIRE(3 == get_index(MyEnum::val3));
  REQUIRE(4 == get_index(MyEnum::val4));
  REQUIRE(5 == get_index(MyEnum::val5));
}

TEST_CASE("enumatic_size", "[enumatic]") {
  REQUIRE(Enumatic<MyEnum>::size() == 6);
}

static_assert(std::is_enum_v<MyEnum>);
static_assert(enumatic::is_enumatic_type<MyEnum>());

static_assert(static_cast<int>(MyEnum::valneg) == -1);
static_assert(static_cast<int>(MyEnum::val1) == 1);
static_assert(static_cast<int>(MyEnum::val2) == 2);
static_assert(static_cast<int>(MyEnum::val3) == 3);
static_assert(static_cast<int>(MyEnum::val5) == -2);
static_assert(Enumatic<MyEnum>::size() == 6);
static_assert(not Enumatic<MyEnum>::has_default_indexation());

TEST_CASE("enumatic_to_from_string", "[enumatic]") { 

  auto val1 = Enumatic<MyEnum>::from_string("val1");
  REQUIRE(val1 == MyEnum::val1); 

  // check with prefix
  auto val2 = Enumatic<MyEnum>::from_string("MyEnum::val2");
  REQUIRE(val2 == MyEnum::val2);

  val2 = Enumatic<MyEnum>::from_string("MyEnum.val2");
  REQUIRE(val2 == MyEnum::val2);

  REQUIRE_THROWS(Enumatic<MyEnum>::from_string("val33") );

  REQUIRE(to_string(MyEnum::val1) == "val1");
  REQUIRE(to_string(MyEnum::val2) == "val2");
  REQUIRE(to_string(MyEnum::val5) == "val5");
}

ENUMATIC_DEFINE(ShortEnum, val1 = 0, val2);
static_assert(Enumatic<ShortEnum>::has_default_indexation());

ENUMATIC_DEFINE(ShortEnum1, val1 = 10, val2 = 255);
static_assert(not Enumatic<ShortEnum1>::has_default_indexation());
ENUMATIC_DEFINE(ShortEnum2, val1 = -1, val2);
static_assert(not Enumatic<ShortEnum2>::has_default_indexation());

enum class OtherEnum { val1, val2 };

static_assert(Enumatic<ShortEnum1>::min_value() == 10);
static_assert(Enumatic<ShortEnum1>::max_value() == 255);

static_assert(Enumatic<ShortEnum2>::min_value() == -1);
static_assert(Enumatic<ShortEnum2>::max_value() == 0);

TEST_CASE("enumatic_serial_size", "[enumatic]") { 
  using namespace app_utils::serial;

  REQUIRE(size(ShortEnum{}) == 2); 
  
  REQUIRE(serial_size(ShortEnum{}) == 1); 
  REQUIRE(serial_size(ShortEnum1{}) == 1);
  REQUIRE(serial_size(ShortEnum2{}) == 4); 

  REQUIRE(serial_size(OtherEnum{}) == 4);
}

TEST_CASE("enumatic_to_buffer", "[enumatic]") {
  auto buffer0 = app_utils::serial::make_buffer(MyEnum::val1);
  auto buffer = app_utils::serial::make_buffer(MyEnum::val1, MyEnum::val3, MyEnum::val2);
  MyEnum val1 = MyEnum::val4, val2 = MyEnum::val4, val3 = MyEnum::val4;
  app_utils::serial::from_bytes(buffer, val1, val2, val3);
  REQUIRE(val1 == MyEnum::val1);
  REQUIRE(val2 == MyEnum::val3);
  REQUIRE(val3 == MyEnum::val2);
}