#include <catch2/catch_test_macros.hpp>

#include <app_utils/serial_utils.hpp>

TEST_CASE("string_to_from_bytes", "[serial]") {
  using namespace app_utils::serial;
  REQUIRE(serial_size("echo") == 5);
  std::string echo_str = "echo";
  std::string_view echo_sv = echo_str;
  REQUIRE(serial_size(echo_str) == 5);
  REQUIRE(serial_size(echo_sv) == 4); // one additional byte for storing the size
}

TEST_CASE("complex_to_from_bytes", "[serial]") {
  using namespace app_utils::serial;
  std::complex<float> val{3, 2};

  REQUIRE(serial_size(val) == 2 * serial_size(val.real()));
  std::vector<std::byte> buffer(10);
  size_t num_bytes_read = to_bytes(buffer, val);

  std::complex<float> val2;
  size_t num_bytes_written = from_bytes(buffer, val2);
  REQUIRE(num_bytes_written == num_bytes_read);
  REQUIRE(val == val2);
}

TEST_CASE("bitset_to_from_bytes", "[serial]") {

  // to-from string for a bitset that fits on more than 1 byte
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

  // to-from string for a bitset that fits on exactly 1 byte
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

  // to-from string for a single bit bitset
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

  // check deserializing to a larger bitset - the intersection should match
  // important for e.g. backward compatibility
  {
    std::bitset<18> u;
    u.set(1);
    u.set(9);
    u.set(17);

    std::vector<std::byte> buffer(10);
    using namespace app_utils::serial;
    REQUIRE(serial_size(u) == 3);
    size_t num_bytes_written = to_bytes(buffer, u);
    buffer.resize(num_bytes_written);
    std::bitset<25> v;
    //v.set();
    size_t num_bytes_read = from_bytes(buffer, v);
    REQUIRE(num_bytes_read == num_bytes_written);
    for(size_t i = 0; i < v.size(); i++) {
      if (i < u.size()) {
        REQUIRE(u.test(i) == v.test(i));
      } else {
        REQUIRE(not v.test(i));
      }
    }
  }

  // check deserializing to a smaller bitset - the intersection should match
  // important for e.g. backward compatibility
  {
    std::bitset<18> u;
    u.set(1);
    u.set(9);
    u.set(17);

    std::vector<std::byte> buffer(10);
    using namespace app_utils::serial;
    REQUIRE(serial_size(u) == 3);
    size_t num_bytes_written = to_bytes(buffer, u);
    buffer.resize(num_bytes_written);
    std::bitset<12> v;
    size_t num_bytes_read = from_bytes(buffer, v);
    REQUIRE(num_bytes_read == 2);
    for(size_t i = 0; i < v.size(); i++) {
      REQUIRE(u.test(i) == v.test(i));
    }
  }
}
