#include <catch2/catch.hpp>
#include <type_traits>

#include <app_utils/enumatic.hpp>
#include <app_utils/serial_utils.hpp>

ENUMATIC_DEFINE(MyEnum, val1, val2,);

TEST_CASE("enumatic_size", "[enumatic]") {
  REQUIRE(Enumatic<MyEnum>::size() == 2);
}

static_assert(std::is_enum_v<MyEnum>);
static_assert(enumatic::is_enumatic_type<MyEnum>());

TEST_CASE("enumatic_to_from_string", "[enumatic]") { 

  auto val1 = Enumatic<MyEnum>::fromString("val1");
  REQUIRE(val1 == MyEnum::val1); 

  // check with prefix
  auto val2 = Enumatic<MyEnum>::fromString("MyEnum::val2");
  REQUIRE(val2 == MyEnum::val2);

  val2 = Enumatic<MyEnum>::fromString("MyEnum.val2");
  REQUIRE(val2 == MyEnum::val2);

  REQUIRE_THROWS(Enumatic<MyEnum>::fromString("val3") );

  REQUIRE(to_string(MyEnum::val1) == "val1");
  REQUIRE(to_string(MyEnum::val2) == "val2");
}

ENUMATIC_DEFINE(ShortEnum, val1, val2);
enum class OtherEnum { val1, val2 };

TEST_CASE("enumatic_serial_size", "[enumatic]") { 
  using namespace app_utils::serial;

  REQUIRE(size(ShortEnum{}) == 2); 
  
  REQUIRE(serial_size(ShortEnum{}) == 1); 

  REQUIRE(serial_size(OtherEnum{}) == 4);
}
