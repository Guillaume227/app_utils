#include <catch2/catch_test_macros.hpp>
#include <type_traits>

#include <app_utils/enumatic.hpp>
#include <app_utils/serial_utils.hpp>
#include <app_utils/serial_buffer.hpp>

namespace {
ENUMATIC_DEFINE(TestEnum, valneg = -1, val1 = 1, val2 = 2, val3, val4 = -3, val5,);
}
static_assert(std::is_standard_layout<TestEnum>());
static_assert(std::is_enum<TestEnum>());

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
  REQUIRE(static_cast<int>(TestEnum::valneg) == -1);
  REQUIRE(static_cast<int>(TestEnum::val1) == 1);
  REQUIRE(static_cast<int>(TestEnum::val2) == 2);
  REQUIRE(static_cast<int>(TestEnum::val3) == 3);
  REQUIRE(static_cast<int>(TestEnum::val4) == -3);
  REQUIRE(static_cast<int>(TestEnum::val5) == -2);
}

TEST_CASE("enumatic::get_underlying_value", "[enumatic]") {
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::valneg) == -1);
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::val1) == 1);
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::val2) == 2);
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::val3) == 3);
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::val4) == -3);
  REQUIRE(Enumatic<TestEnum>::get_underlying_value(TestEnum::val5) == -2);
}

TEST_CASE("enumatic::get_values", "[enumatic]") {
  auto const vals_array = Enumatic<TestEnum>::get_values();

  REQUIRE(vals_array[0] == TestEnum::valneg);
  REQUIRE(vals_array[1] == TestEnum::val1);
  REQUIRE(vals_array[2] == TestEnum::val2);
  REQUIRE(vals_array[3] == TestEnum::val3);
  REQUIRE(vals_array[4] == TestEnum::val4);
  REQUIRE(vals_array[5] == TestEnum::val5);
}

TEST_CASE("enumatic::get_index", "[enumatic]") {
  REQUIRE(0 == get_index(TestEnum::valneg));
  REQUIRE(1 == get_index(TestEnum::val1));
  REQUIRE(2 == get_index(TestEnum::val2));
  REQUIRE(3 == get_index(TestEnum::val3));
  REQUIRE(4 == get_index(TestEnum::val4));
  REQUIRE(5 == get_index(TestEnum::val5));
}

TEST_CASE("enumatic_size", "[enumatic]") {
  REQUIRE(Enumatic<TestEnum>::size() == 6);
}

static_assert(std::is_enum_v<TestEnum>);
static_assert(enumatic::is_enumatic_type<TestEnum>());

static_assert(static_cast<int>(TestEnum::valneg) == -1);
static_assert(static_cast<int>(TestEnum::val1) == 1);
static_assert(static_cast<int>(TestEnum::val2) == 2);
static_assert(static_cast<int>(TestEnum::val3) == 3);
static_assert(static_cast<int>(TestEnum::val5) == -2);
static_assert(Enumatic<TestEnum>::size() == 6);
static_assert(not Enumatic<TestEnum>::has_default_indexation());

TEST_CASE("enumatic_to_from_string", "[enumatic]") { 

  auto val1 = Enumatic<TestEnum>::from_string("val1");
  REQUIRE(val1 == TestEnum::val1);

  // check with prefix
  auto val2 = Enumatic<TestEnum>::from_string("TestEnum::val2");
  REQUIRE(val2 == TestEnum::val2);

  val2 = Enumatic<TestEnum>::from_string("TestEnum.val2");
  REQUIRE(val2 == TestEnum::val2);

  REQUIRE_THROWS(Enumatic<TestEnum>::from_string("val33") );

  REQUIRE(to_string(TestEnum::val1) == "val1");
  REQUIRE(to_string(TestEnum::val2) == "val2");
  REQUIRE(to_string(TestEnum::val5) == "val5");
}
namespace {
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
}

TEST_CASE("enumatic_serial_size", "[enumatic]") { 
  using namespace app_utils::serial;

  REQUIRE(size(ShortEnum{}) == 2); 
  
  REQUIRE(serial_size(ShortEnum{}) == 1); 
  REQUIRE(serial_size(ShortEnum1{}) == 1);
  REQUIRE(serial_size(ShortEnum2{}) == 4); 

  REQUIRE(serial_size(OtherEnum{}) == 4);
}

TEST_CASE("enumatic_to_buffer", "[enumatic]") {
  auto buffer0 = app_utils::serial::make_buffer(TestEnum::val1);
  auto buffer = app_utils::serial::make_buffer(TestEnum::val1, TestEnum::val3, TestEnum::val2);
  TestEnum val1 = TestEnum::val4, val2 = TestEnum::val4, val3 = TestEnum::val4;
  app_utils::serial::from_bytes(buffer, val1, val2, val3);
  REQUIRE(val1 == TestEnum::val1);
  REQUIRE(val2 == TestEnum::val3);
  REQUIRE(val3 == TestEnum::val2);
}
