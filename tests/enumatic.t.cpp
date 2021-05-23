#define CATCH_CONFIG_MAIN
#include <catch2/catch.hpp>


#include <app_utils/enumatic.hpp>


ENUMATIC_DEFINE(Toto, val1, val2);

TEST_CASE("enum_size", "[enumatic]") {
  REQUIRE(Enumatic<Toto>::size() == 2);
}
