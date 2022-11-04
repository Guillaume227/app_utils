#include <catch2/catch_test_macros.hpp>

#include <app_utils/circular_vector.hpp>

#include <type_traits>

// LegacyForwardIterator requirements
static_assert(std::is_copy_constructible_v<circular_vector_t<int>::Iterator>);
static_assert(std::is_default_constructible_v<circular_vector_t<int>::Iterator>);
static_assert(std::is_copy_assignable_v<circular_vector_t<int>::Iterator>);
static_assert(std::is_swappable_v<circular_vector_t<int>::Iterator>);
static_assert(std::is_same_v<circular_vector_t<int>::Iterator::difference_type, std::ptrdiff_t>);

TEST_CASE("circular_vector", "[container]") {
  circular_vector_t<int> buff(3);

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
  REQUIRE(std::count(buff.begin(), buff.end(), 0) == 0);
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