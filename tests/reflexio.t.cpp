#include <catch2/catch.hpp>

#include <app_utils/reflexio.hpp>

#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

#if defined(_MSC_VER) && _MSC_VER >= 1929
#define CONSTEXPR_STRING_AND_VECTOR 
#endif

ENUMATIC_DEFINE(TestEnum, EnumVal1, EnumVal2);

static_assert(sizeof(TestEnum) == 4);

REFLEXIO_STRUCT_DEFINE(MiniStruct, REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 14, "var1 doc");
                       static_assert(member_var_counter_t<var1_id>::index == 0);
                       static_assert(member_var_descriptor_t<0, int>::get_descriptor() != nullptr););

TEST_CASE("mini_struct", "[reflexio]") {
  
  MiniStruct miniStruct;
  REQUIRE(miniStruct.has_all_default_values());
}

REFLEXIO_STRUCT_DEFINE(MyStruct,   
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(TestEnum, var3, TestEnum::EnumVal2, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var4, true, "var5 doc");
  using Array8_t = std::array<float, 8>;
  REFLEXIO_MEMBER_VAR_DEFINE(Array8_t, var5, {0}, "var6 doc");
  static_assert(member_var_counter_t<var2_id>::index == 1);
  static_assert(member_var_descriptor_t<3, int>::get_descriptor() != nullptr);
  );

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, .5f, "var2 doc"););


REFLEXIO_STRUCT_DEFINE(TrivialStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.1f, "var2 doc"););

static_assert(std::is_standard_layout_v<TrivialStruct>);
static_assert(std::is_trivially_copyable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_assignable_v<TrivialStruct>);
static_assert(std::is_trivially_copy_constructible_v<TrivialStruct>);
//static_assert(std::is_trivially_constructible_v<TrivialStruct>); // NO: because of in-line initialization

TEST_CASE("reflexio_declare", "[reflexio]") { 

  size_t const num_members = 5;

  REQUIRE(MyStruct::num_members() == num_members);

  MyStruct myStruct; 
  
  REQUIRE(MyStruct::num_members() == num_members);

  MyStruct myStruct2;

  REQUIRE(MyStruct::num_members() == num_members);

  MyOtherStruct myOtherStruct;

  REQUIRE(MyStruct::num_members() == num_members);

  REQUIRE(myStruct == myStruct2);
  REQUIRE(myStruct.has_all_default_values());
  myStruct.var5[2] = 12;
  REQUIRE(not myStruct.has_all_default_values());
  REQUIRE(myStruct != myStruct2);

  REQUIRE(MyStruct::get_member_descriptors()[0]->default_value_as_string() == "12");
  REQUIRE(MyStruct::get_member_descriptors()[1]->default_value_as_string() == "1.500000");
  REQUIRE(MyStruct::get_member_descriptors()[2]->default_value_as_string() == "EnumVal2");
  REQUIRE(MyStruct::get_member_descriptors()[3]->default_value_as_string() == "true"); // TODO: should be "true"
}

TEST_CASE("reflexio_serialize", "[reflexio]") {

  TrivialStruct trivialStruct;
  REQUIRE(trivialStruct.get_serial_size() == sizeof(float) + sizeof(int));

  REQUIRE(serial_size(trivialStruct) == sizeof(float) + sizeof(int));

  MyStruct sendStruct;
  sendStruct.var1 = 2;
  sendStruct.var2 = 0.5;
  sendStruct.var3 = TestEnum::EnumVal1;
  sendStruct.var4 = false;
  sendStruct.var5 = {1.f, 2.f, 3.f, 6.f, 5.f, 4.f, 7.f, 8.f};
  MyStruct receiveStruct;
  REQUIRE(sendStruct != receiveStruct);
  std::vector<std::byte> buffer(256);
  
  size_t const written_bytes = to_bytes(buffer.data(), buffer.size(), sendStruct);

  size_t const expected_serial_size = sizeof(int) + sizeof(float) +
                                      /*TestEnum*/ 1 + /*bool*/ 1 +
                                      8 * sizeof(float);
  
  REQUIRE(written_bytes == expected_serial_size);

  from_bytes(buffer.data(), written_bytes, receiveStruct);

  REQUIRE(sendStruct == receiveStruct);
}


#ifdef CONSTEXPR_STRING_AND_VECTOR
REFLEXIO_STRUCT_DEFINE(StructWithStringAndVector, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var6, "var2_val", "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::vector<float>, var7, {}, "var7 doc"););


TEST_CASE("reflexio_string_and_vector", "[reflexio]") {

  StructWithStringAndVector sendStruct;
  sendStruct.var1 = 2;
  sendStruct.var6 = "new_val";
  sendStruct.var7 = {1.f, 2.f, 3.f};

  REQUIRE(StructWithStringAndVector::get_member_descriptors()[1]->default_value_as_string() == "var2_val");

  StructWithStringAndVector receiveStruct;
  REQUIRE(sendStruct != receiveStruct);
  std::vector<std::byte> buffer(256);

  size_t const written_bytes = to_bytes(buffer.data(), buffer.size(), sendStruct);

  size_t const expected_serial_size = sizeof(int) + /*string*/ 8 + /*vector*/ 3 * sizeof(float) + 1;

  REQUIRE(written_bytes == expected_serial_size);

  from_bytes(buffer.data(), written_bytes, receiveStruct);

  REQUIRE(sendStruct == receiveStruct);
}

#endif
