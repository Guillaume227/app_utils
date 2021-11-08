#include <catch2/catch.hpp>


#include <app_utils/string_utils.hpp>

using namespace app_utils::strutils;

TEST_CASE("split", "[string_utils]") {
{
  std::string_view valStr = " Bonjour, tristesse ";
  auto res = split(',', valStr);
  REQUIRE(res.size() == 2);
  REQUIRE(res[0] == "Bonjour");
  REQUIRE(res[1] == "tristesse");
}

{ // do not strip whitespace
  std::string_view valStr = " Bonjour, tristesse ";
  auto res = split(',', valStr, /*stripWhiteSpace*/ false);
  REQUIRE(res.size() == 2);
  REQUIRE(res[0] == " Bonjour");
  REQUIRE(res[1] == " tristesse ");
}

{
  std::string_view valStr = "1 2 3 ";
  auto res = split(' ', valStr);
  REQUIRE(res.size() == 3);
  REQUIRE(res[0] == "1");
  REQUIRE(res[1] == "2");
  REQUIRE(res[2] == "3");
}

{ // Limited number of split
  std::string_view valStr = "1 2 3 ";
  auto res = split(' ', valStr, /*stripWhiteSpace*/true, /*nSplit*/1);
  REQUIRE(res.size() == 2);
  REQUIRE(res[0] == "1");
  REQUIRE(res[1] == "2 3 ");
}

{ // Limited number of split
  std::string_view valStr = "1 2 3 ";
  auto res = split(' ', valStr, /*stripWhiteSpace*/true, /*nSplit*/10);
  REQUIRE(res.size() == 3);
  REQUIRE(res[0] == "1");
  REQUIRE(res[1] == "2");
  REQUIRE(res[2] == "3");
}
}
