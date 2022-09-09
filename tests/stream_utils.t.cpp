#include <catch2/catch_test_macros.hpp>

#include <app_utils/stream_utils.hpp>
#include <app_utils/log_utils.hpp>
#include <vector>

TEST_CASE("formatting", "[stream]") {

  std::string_view some_chars = "some chars";
  auto result = app_utils::make_string("Formatted sample '", 1.33, "'",
        ";", some_chars, ", (", 1,
                                                          ",", 2, ", ", 3, ").", "[", 1, 2, 3, "]");
  REQUIRE(result == "Formatted sample '1.33'; some chars, (1, 2, 3). [1 2 3]");
}


TEST_CASE("formatting_bytes", "[stream]") {

  {
    std::ostringstream oss;
    std::byte ab {0xAB};
    oss << std::hex << (uint8_t)ab;
    std::string output = oss.str();
    LOG_LINE(output, ab);
    REQUIRE(output == "\xAB");

  }
  {
    std::string output = app_utils::make_string(std::byte{0xAB});
    REQUIRE(output == "\xAB");
  }
  {
    std::vector<std::byte> bytes{std::byte{0xFF}, std::byte{0xAA}, std::byte{0xBB}, std::byte{0x11}};
    {
      std::string output = app_utils::make_string(bytes);
      LOG_LINE(output, bytes);
      REQUIRE(output == "\xFF\xAA\xBB\x11");
    }
    {
      std::span vals_span = bytes;
      std::string output = app_utils::make_string(vals_span.first(4));
      LOG_LINE(output, vals_span);
      REQUIRE(output == "\xFF\xAA\xBB\x11");
    }
  }
  {
    std::byte bytes[4] {std::byte{0xFF}, std::byte{0xAA}, std::byte{0xBB}, std::byte{0x11}};
    std::span vals_span = bytes;
    std::string output = app_utils::make_string(vals_span);
    LOG_LINE(output, vals_span);
    REQUIRE(output == "\xFF\xAA\xBB\x11");
  }
}