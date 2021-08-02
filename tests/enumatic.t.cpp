#include <catch2/catch.hpp>


#include <app_utils/enumatic.hpp>


ENUMATIC_DEFINE(Toto, val1, val2);

TEST_CASE("enum_size", "[enumatic]") {
  REQUIRE(Enumatic<Toto>::size() == 2);
}


TEST_CASE("fromString", "[enumatic]") { 

  auto val1 = Enumatic<Toto>::fromString("val1");
  REQUIRE(val1 == Toto::val1); 

  auto val2 = Enumatic<Toto>::fromString("val2");
  REQUIRE(val2 == Toto::val2);
  
  REQUIRE_THROWS(Enumatic<Toto>::fromString("val3") );
}
