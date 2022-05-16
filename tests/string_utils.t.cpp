#include <catch2/catch.hpp>


#include <app_utils/string_utils.hpp>

using namespace app_utils::strutils;

TEST_CASE("strutils::split", "[string_utils]") {
  {
    std::string_view valStr = " Bonjour, tristesse ";
    auto res = split(',', valStr);
    REQUIRE(res.size() == 2);
    REQUIRE(res[0] == "Bonjour");
    REQUIRE(res[1] == "tristesse");
  }

  {// do not strip whitespace
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

  {// Limited number of split
    std::string_view valStr = "1 2 3 ";
    auto res = split(' ', valStr, /*stripWhiteSpace*/ true, /*nSplit*/ 1);
    REQUIRE(res.size() == 2);
    REQUIRE(res[0] == "1");
    REQUIRE(res[1] == "2 3 ");
  }

  {// Limited number of split
    std::string_view valStr = "1 2 3 ";
    auto res = split(' ', valStr, /*stripWhiteSpace*/ true, /*nSplit*/ 10);
    REQUIRE(res.size() == 3);
    REQUIRE(res[0] == "1");
    REQUIRE(res[1] == "2");
    REQUIRE(res[2] == "3");
  }
}

TEST_CASE("strutils::split_parse", "[string_utils]") {
  {
    std::string_view valStr = "  (1, {2}), ({3, 4}) ";
    auto res = splitParse(valStr, ',');
    REQUIRE(res.size() == 2);
    REQUIRE(res[0] == "(1, {2})");
    REQUIRE(res[1] == "({3, 4})");
  }
}

TEST_CASE("strutils::from_string", "[string_utils]") {
  using namespace app_utils::strutils;
  float f;
  REQUIRE_THROWS(from_string(f, "11.11 asldk"));

  REQUIRE_THROWS(from_string(f, "asdf 11.11"));

  from_string(f, "11.11");
  REQUIRE(f == 11.11f);

  double d;
  from_string(d, "11.11");
  REQUIRE(d == 11.11);
}
