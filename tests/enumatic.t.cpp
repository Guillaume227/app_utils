#include <catch2/catch.hpp>
#include <type_traits>

#include <app_utils/enumatic.hpp>
#include <app_utils/serial_utils.hpp>

ENUMATIC_DEFINE(MyEnum, valneg = -1, val1 = 1, val2 = 2, val3, val4 = -3, val5,);


TEST_CASE("parse_enum", "[enumatic]") { 
  auto enum_details = enumatic::parseEnumDefinition<4>("valneg [[deprecated]]=  -1, val1 = 1, val2 [[deprecated]], val3  , "); 
  REQUIRE(enum_details[0].value_name == "valneg");
  REQUIRE(enum_details[1].value_name == "val1");
  REQUIRE(enum_details[2].value_name == "val2");
  REQUIRE(enum_details[3].value_name == "val3");

  REQUIRE(enum_details[0].int_value == -1);
  REQUIRE(enum_details[1].int_value == 1);  
  REQUIRE(enum_details[2].int_value == 2);
  REQUIRE(enum_details[3].int_value == 3);
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

  auto val1 = Enumatic<MyEnum>::fromString("val1");
  REQUIRE(val1 == MyEnum::val1); 

  // check with prefix
  auto val2 = Enumatic<MyEnum>::fromString("MyEnum::val2");
  REQUIRE(val2 == MyEnum::val2);

  val2 = Enumatic<MyEnum>::fromString("MyEnum.val2");
  REQUIRE(val2 == MyEnum::val2);

  REQUIRE_THROWS(Enumatic<MyEnum>::fromString("val33") );

  REQUIRE(to_string(MyEnum::val1) == "val1");
  REQUIRE(to_string(MyEnum::val2) == "val2");
  REQUIRE(to_string(MyEnum::val5) == "val5");
}

ENUMATIC_DEFINE(ShortEnum, val1, val2);
enum class OtherEnum { val1, val2 };

static_assert(Enumatic<ShortEnum>::has_default_indexation());

TEST_CASE("enumatic_serial_size", "[enumatic]") { 
  using namespace app_utils::serial;

  REQUIRE(size(ShortEnum{}) == 2); 
  
  REQUIRE(serial_size(ShortEnum{}) == 1); 

  REQUIRE(serial_size(OtherEnum{}) == 4);
}
