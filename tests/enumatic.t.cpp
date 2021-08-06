#include <catch2/catch.hpp>
#include <type_traits>

#include <app_utils/enumatic.hpp>


ENUMATIC_DEFINE(Toto, val1, val2,);

TEST_CASE("enum_size", "[enumatic]") {
  REQUIRE(Enumatic<Toto>::size() == 2);
}


static_assert(std::is_enum_v<Toto>);
//static_assert(enumatic::is_enumatic_type<Toto>());


TEST_CASE("to_fromString", "[enumatic]") { 

  auto val1 = Enumatic<Toto>::fromString("val1");
  REQUIRE(val1 == Toto::val1); 

  auto val2 = Enumatic<Toto>::fromString("val2");
  REQUIRE(val2 == Toto::val2);
  
  REQUIRE_THROWS(Enumatic<Toto>::fromString("val3") );

  REQUIRE(to_string(Toto::val1) == "val1");
  REQUIRE(to_string(Toto::val2) == "val2");
}
