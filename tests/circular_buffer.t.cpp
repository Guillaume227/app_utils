#include <catch2/catch_test_macros.hpp>


#include <app_utils/circular_buffer.hpp>
#include <app_utils/circular_buffer_flex.hpp>


TEST_CASE("circular_buffer", "[container]") {

  app_utils::circular_buffer_t<int, 3> buffer;

  std::vector<int> test_vec;

  REQUIRE(buffer.capacity() == 3);
  REQUIRE(buffer.size() == 0);

  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 0);

  buffer.get_next_slot() = 1;
  buffer.get_next_slot() = 2;
  REQUIRE(buffer.size() == 2);
  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 2);
  REQUIRE(test_vec[0] == 1);
  REQUIRE(test_vec[1] == 2);
  test_vec.clear();

  buffer.get_next_slot() = 3;
  buffer.get_next_slot() = 4;

  REQUIRE(buffer.size() == 3);
  for (auto& item : buffer) {
    test_vec.push_back(item);
  }
  REQUIRE(test_vec.size() == 3);
  REQUIRE(test_vec[0] == 2);
  REQUIRE(test_vec[1] == 3);
  REQUIRE(test_vec[2] == 4);
}

TEST_CASE("circular_buffer_flex", "[container]") {
  circular_buffer_flex_t<int> buff(3);

  auto calc_avg = [](auto const& buffer)->int{
    int sum = 0;
    for (auto& val: buffer) {
      sum += val;
    }
    if (buffer.empty()) {
      return 0;
    }
    return sum / (int)buffer.size();
  };

  REQUIRE(buff.size() == 0);
  REQUIRE(buff.empty());
  REQUIRE(calc_avg(buff) == 0);

  buff.push_back(2);
  buff.push_back(4);

  REQUIRE(buff.size() == 2);
  REQUIRE(buff.get_front_index() == 0);

  REQUIRE(calc_avg(buff) == 3);

  buff.push_back(6);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(calc_avg(buff) == 4);

  buff.push_back(8);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 1);
  REQUIRE(calc_avg(buff) == 6);

  buff.push_back(6);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 2);
  REQUIRE(calc_avg(buff) == 6); // 20 / 3 rounds down to 6

  buff.push_back(4);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(calc_avg(buff) == 6);
}