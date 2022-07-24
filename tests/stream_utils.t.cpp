#include <catch2/catch_test_macros.hpp>

#include <app_utils/stream_utils.hpp>


TEST_CASE("formatting", "[stream]") {

  std::string_view some_chars = "some chars";
  auto result = app_utils::stream::StreamWriter::writeStr("Formatted sample '", 1.33, "'", 
        ";", some_chars, ", (", 1,
                                                          ",", 2, ", ", 3, ").", "[", 1, 2, 3, "]");
  REQUIRE(result == "Formatted sample '1.33'; some chars, (1, 2, 3). [1 2 3]");
}
