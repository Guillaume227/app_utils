#include <catch2/catch.hpp>

#include <app_utils/time_utils.hpp>

using namespace std::chrono_literals;

TEST_CASE("durationFromString", "[time_utils]") { 
  
  REQUIRE(12ns == app_utils::time::durationFromStr<std::chrono::nanoseconds>("12ns"));
  REQUIRE_THROWS(app_utils::time::durationFromStr("12ns"));// lossy conversion from nanoseconds to seconds

  REQUIRE_THROWS(app_utils::time::durationFromStr<std::chrono::nanoseconds>("12")); //missing units
}


TEST_CASE("durationToString", "[time_utils]") {
  REQUIRE("2h3min38s10ms1us" == app_utils::time::formatDuration(1h + 61min + 158s + 10'001us));
}
