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

  auto to_vector = [](auto const& buffer) {
    std::vector<int> res;
    for (auto elem: buffer) {
      res.push_back(elem);
    }
    return res;
  };

  REQUIRE(buff.size() == 0);
  REQUIRE(std::count(buff.begin(), buff.end(), 0) == 0);
  REQUIRE(buff.empty());
  REQUIRE(calc_avg(buff) == 0);

  buff.push_back(2);

  REQUIRE(buff.size() == 1);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 0);
  REQUIRE(to_vector(buff) == std::vector<int>{2});

  buff.push_back(4);

  REQUIRE(buff.size() == 2);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 1);
  REQUIRE(to_vector(buff) == std::vector<int>{2, 4});

  REQUIRE(calc_avg(buff) == 3);

  buff.push_back(6);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 2);
  REQUIRE(calc_avg(buff) == 4);
  REQUIRE(to_vector(buff) == std::vector<int>{2, 4, 6});

  buff.push_back(8);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 1);
  REQUIRE(buff.get_back_index() == 0);
  REQUIRE(calc_avg(buff) == 6);
  REQUIRE(to_vector(buff) == std::vector<int>{4, 6, 8});

  buff.push_back(6);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 2);
  REQUIRE(buff.get_back_index() == 1);
  REQUIRE(calc_avg(buff) == 6); // 20 / 3 rounds down to 6
  REQUIRE(to_vector(buff) == std::vector<int>{6, 8, 6});

  buff.push_back(4);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 2);
  REQUIRE(calc_avg(buff) == 6);
  REQUIRE(to_vector(buff) == std::vector<int>{8, 6, 4});

  buff.push_back(4);
  REQUIRE(buff.size() == 3);
  REQUIRE(buff.get_front_index() == 1);
  REQUIRE(buff.get_back_index() == 0);
  REQUIRE(to_vector(buff) == std::vector<int>{6, 4, 4});

  buff.pop_back();
  REQUIRE(buff.size() == 2);
  REQUIRE(buff.get_front_index() == 1);
  REQUIRE(buff.get_back_index() == 2);
  REQUIRE(to_vector(buff) == std::vector<int>{6, 4});

  auto buff2 = buff;
  buff2.pop_front();
  REQUIRE(buff2.size() == 1);
  REQUIRE(buff2.get_front_index() == 2);
  REQUIRE(buff2.get_back_index() == 2);
  REQUIRE(to_vector(buff2) == std::vector<int>{4});

  buff.pop_back();
  REQUIRE(buff.size() == 1);
  REQUIRE(buff.get_front_index() == 1);
  REQUIRE(buff.get_back_index() == 1);
  REQUIRE(to_vector(buff) == std::vector<int>{6});

  buff.pop_back();
  REQUIRE(buff.size() == 0);
  REQUIRE(buff.empty());
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 0);

  // pop back on an empty buffer
  buff.pop_back();
  REQUIRE(buff.size() == 0);
  REQUIRE(buff.empty());
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 0);

  // pop front on an empty buffer
  buff.pop_front();
  REQUIRE(buff.size() == 0);
  REQUIRE(buff.empty());
  REQUIRE(buff.get_front_index() == 0);
  REQUIRE(buff.get_back_index() == 0);

  buff2.pop_front();
  REQUIRE(buff2.size() == 0);
  REQUIRE(buff2.empty());
  REQUIRE(buff2.get_front_index() == 0);
  REQUIRE(buff2.get_back_index() == 0);
}