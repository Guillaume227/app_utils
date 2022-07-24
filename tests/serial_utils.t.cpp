#include <catch2/catch_test_macros.hpp>

#include <app_utils/serial_utils.hpp>


TEST_CASE("bitset_to_from_bytes", "[serial]") {

  {
    std::bitset<10> u;
    u.set(3);
    u.set(8);

    std::vector<std::byte> buffer(10);
    using namespace app_utils::serial;
    REQUIRE(serial_size(u) == 2);
    size_t num_bytes_written = to_bytes(buffer, u);
    buffer.resize(num_bytes_written);
    decltype(u) v;
    REQUIRE(u != v);
    from_bytes(buffer, v);
    REQUIRE(u == v);
  }

  {
    std::bitset<8> u;
    u.set(3);
    u.set(5);

    std::vector<std::byte> buffer(10);
    using namespace app_utils::serial;
    REQUIRE(serial_size(u) == 1);
    size_t num_bytes_written = to_bytes(buffer, u);
    buffer.resize(num_bytes_written);
    decltype(u) v;
    REQUIRE(u != v);
    from_bytes(buffer, v);
    REQUIRE(u == v);
  }

  {
    std::bitset<1> u;
    u.set(0);

    std::vector<std::byte> buffer(10);
    using namespace app_utils::serial;
    REQUIRE(serial_size(u) == 1);
    size_t num_bytes_written = to_bytes(buffer, u);
    buffer.resize(num_bytes_written);
    decltype(u) v;
    REQUIRE(u != v);
    from_bytes(buffer, v);
    REQUIRE(u == v);
  }
}
