#include <catch2/catch.hpp>


#include <app_utils/circular_buffer.hpp>


TEST_CASE("circular_buffer", "[container]") {

  app_utils::circular_buffer_t<int, 3> buffer;

  std::vector<int> test_vec;

  REQUIRE(buffer.capacity() == 3);
  REQUIRE(buffer.size() == 0);

  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 0);

  buffer.push_back(1);
  buffer.push_back(2);
  REQUIRE(buffer.size() == 2);
  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 2);
  REQUIRE(test_vec[0] == 1);
  REQUIRE(test_vec[1] == 2);
  test_vec.clear();

  buffer.push_back(3);
  buffer.push_back(4);

  REQUIRE(buffer.size() == 3);
  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 3);
  REQUIRE(test_vec[0] == 2);
  REQUIRE(test_vec[1] == 3);
  REQUIRE(test_vec[2] == 4);
}
