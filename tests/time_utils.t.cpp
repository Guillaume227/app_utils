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

  auto const duration = 131h + 61min + 158s + 10'001us;
  std::string_view str_value = "132h  3min 38s 10ms  1us";
  REQUIRE(app_utils::time::formatDuration(duration) == str_value);


  std::ostringstream oss;
  oss << duration;
  REQUIRE(oss.str() == "475418010001us");

  REQUIRE(app_utils::stream::StreamWriter::writeStr(duration) == str_value);
}
