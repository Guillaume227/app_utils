#include <catch2/catch_test_macros.hpp>

#include <app_utils/stream_utils.hpp>
#include <app_utils/time_utils.hpp>

using namespace std::chrono_literals;

TEST_CASE("durationFromString", "[time_utils]") {

  REQUIRE(12ns == app_utils::time::durationFromStr<std::chrono::nanoseconds>("12ns"));
  REQUIRE_THROWS(app_utils::time::durationFromStr("12ns"));// lossy conversion from nanoseconds to seconds

  REQUIRE_THROWS(app_utils::time::durationFromStr<std::chrono::nanoseconds>("12")); //missing units
}

TEST_CASE("durationToString", "[time_utils]") {

  REQUIRE(app_utils::time::formatDuration(3min) == "3min");
  REQUIRE(app_utils::time::formatDuration(3ms) == "3ms");

  REQUIRE(app_utils::time::formatDuration(3s) == "3s");
  REQUIRE(app_utils::time::formatDuration(1min+3s) == "1min3s");
  REQUIRE(app_utils::time::formatDuration(63s) == "1min3s");
  REQUIRE(app_utils::time::formatDuration(1h+3s) == "1h3s");
  REQUIRE(app_utils::time::formatDuration(2h+3min) == "2h3min");
  REQUIRE(app_utils::time::formatDuration(1h+63min) == "2h3min");

  auto const duration = 131h + 61min + 158s + 10'001us;
  std::string_view str_value = "132h3min38s10ms1us";
  REQUIRE(app_utils::time::formatDuration(duration) == str_value);


  std::ostringstream oss;
  oss << duration;
  REQUIRE(oss.str() == "475418010001us");

  REQUIRE(app_utils::make_string(duration) == str_value);
}
