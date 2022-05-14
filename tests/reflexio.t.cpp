#include <catch2/catch.hpp>

#include <app_utils/reflexio.hpp>

#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

#include "reflexio.t.hpp"

TEST_CASE("reflexio_single_var_struct", "[reflexio]") {
  SingleVarStruct singleVarStruct;
  REQUIRE(singleVarStruct.has_all_default_values());
  REQUIRE(to_string(singleVarStruct) == "var1: " + std::to_string(singleVarStruct.var1) + "\n");
}

REFLEXIO_STRUCT_DEFINE(MyStruct,
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var2, 1.5f, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(TestEnum, var3, TestEnum::EnumVal2, "var4 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(bool, var4, true, "var5 doc");
  int normal_member = 10; // a non registered member
  using Array8_t = std::array<float, 8>;
  REFLEXIO_MEMBER_VAR_DEFINE(Array8_t, var5, {0}, "var6 doc");
  Array8_t normal_var;
  static_assert(member_var_counter_t<__var2_id>::index == 1);
  static_assert(member_var_traits_t<3, int>::descriptor != nullptr);
  );

static_assert(std::is_standard_layout<MyStruct>());
static_assert(std::is_trivially_copyable<MyStruct>());

TEST_CASE("reflexio_default_values", "[reflexio]") {
  TrivialStruct myStruct;
  REQUIRE(myStruct.has_all_default_values());
  myStruct.var2 = 7;
  REQUIRE(not myStruct.has_all_default_values());
}

TEST_CASE("reflexio_mask", "[reflexio]") {
  TrivialStruct myStruct;
  auto mask_var1 = TrivialStruct::make_vars_mask(&TrivialStruct::var1);
  REQUIRE(mask_var1 == TrivialStruct::MemberVarsMask{2});
  auto mask_var2 = TrivialStruct::make_vars_mask(&TrivialStruct::var2);
  REQUIRE(mask_var2 == TrivialStruct::MemberVarsMask{1});

  auto mask_var_all = TrivialStruct::make_vars_mask(&TrivialStruct::var1,
                                                    &TrivialStruct::var2);
  REQUIRE(mask_var_all == TrivialStruct::MemberVarsMask{});
}

TEST_CASE("reflexio_declare", "[reflexio]") {

  size_t const expected_member_vars = 5;

  REQUIRE(MyStruct::num_registered_member_vars() == expected_member_vars);

  MyStruct myStruct; 
  
  REQUIRE(MyStruct::num_registered_member_vars() == expected_member_vars);

  MyStruct myStruct2;

  REQUIRE(MyStruct::num_registered_member_vars() == expected_member_vars);

  MyOtherStruct myOtherStruct;

  REQUIRE(MyStruct::num_registered_member_vars() == expected_member_vars);
  myStruct.normal_member = 11;
  REQUIRE(myStruct == myStruct2);
  REQUIRE(myStruct.normal_member != myStruct2.normal_member);
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
  REQUIRE(trivialStruct.get_serial_size() == sizeof(float) + sizeof(int8_t));

  REQUIRE(serial_size(trivialStruct) == sizeof(float) + sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::MemberVarsMask{0}) == sizeof(float) + sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::MemberVarsMask{1}) == sizeof(float));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::MemberVarsMask{2}) == sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::MemberVarsMask{3}) == 0);

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

TEST_CASE("reflexio_constexpr", "[reflexio]") {
  constexpr TrivialStruct myStruct_const;

  constexpr size_t myStruct_size = myStruct_const.get_serial_size();
  REQUIRE(myStruct_size == sizeof(float) + sizeof(int8_t));

  constexpr TrivialStruct::MemberVarsMask filter{1};
  constexpr size_t myStruct_size_1 = myStruct_const.get_serial_size(filter);
  REQUIRE(myStruct_size_1 == sizeof(float));

  // The below won't work because of the static_cast in reflexio::get_value,
  // which the language doesn't allow in a constexpr context.
  /*
  static_assert(myStruct_const.has_all_default_values());
  static_assert(myStruct_const.non_default_values().empty());
  constexpr std::string repr = to_string(myStruct_const);
  static_assert(not repr.empty());
 */
}

TEST_CASE("reflexio_iterator", "[reflexio]") {
  std::ostringstream oss;
  SingleVarStruct SingleVarStruct;
  for (auto& descriptor : SingleVarStruct) {
    oss << descriptor.get_name() << ";";
  }
  REQUIRE(oss.str() == "var1;");
}

TEST_CASE("reflexio_from_string", "[reflexio]") {

  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(var1: 11
    var2: 22.22)";
    from_string(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    refStruct.var2 = 22.22;
    REQUIRE(reflexioStruct == refStruct);

    std::string valstr = to_string(reflexioStruct);
    reflexioStruct.var2 = 33.33;
    from_string(reflexioStruct, valstr);

    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(   var1: 11      )";
    from_string(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    REQUIRE(reflexioStruct == refStruct);
  }
  {// with no extra new lines
    TrivialStruct reflexioStruct;
    std::string_view val_str = "var1: 11";
    from_string(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(var2: 22.22)";
    from_string(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var2 = 22.22;
    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = "";
    from_string(reflexioStruct, val_str);
    TrivialStruct refStruct;
    REQUIRE(reflexioStruct == refStruct);
  }
}

TEST_CASE("reflexio_composite", "[reflexio]") {

  NestedStruct myStruct;
  REQUIRE(myStruct.has_all_default_values());
  myStruct.var1.var1 = 2;

  REQUIRE(not myStruct.has_all_default_values());

  auto myStructCopy = myStruct;
  REQUIRE(myStructCopy == myStruct);
  auto& subStruct = myStructCopy.var1;
  subStruct.var2 = 22.f;

  REQUIRE(myStructCopy != myStruct);

  /*
  auto val_str = to_string(myStruct);
  std::cout << "Value:" << std::endl << val_str << std::endl << "endvalue" << std::endl;
  from_string(myStructCopy, val_str);

  REQUIRE(myStructCopy == myStruct);
  */
}

#ifdef CONSTEXPR_STRING_AND_VECTOR
REFLEXIO_STRUCT_DEFINE(StructWithStringAndVector, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var6, "var2_val", "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::vector<float>, var7, {10.f}, "var7 doc"););


TEST_CASE("reflexio_string_and_vector", "[reflexio]") {

  StructWithStringAndVector sendStruct;
  sendStruct.var1 = 2;
  sendStruct.var6 = "new_val";
  sendStruct.var7 = {1.f, 2.f, 3.f};

  REQUIRE(StructWithStringAndVector::get_member_descriptors()[0]->default_value_as_string() == "12");
  REQUIRE(StructWithStringAndVector::get_member_descriptors()[1]->default_value_as_string() == "var2_val");
  // transient constexpr allocation: see comment in reflexio.hpp around reflexio_traits::DefaultType specialization
  //REQUIRE(StructWithStringAndVector::get_member_descriptors()[2]->default_value_as_string() == "{10.f}");

  StructWithStringAndVector receiveStruct;
  REQUIRE(sendStruct != receiveStruct);
  std::vector<std::byte> buffer(256);

  size_t const written_bytes = to_bytes(buffer.data(), buffer.size(), sendStruct);

  size_t const expected_serial_size = sizeof(int) + /*string*/ 8 + /*vector*/ 3 * sizeof(float) + 1;

  REQUIRE(written_bytes == expected_serial_size);

  from_bytes(buffer.data(), written_bytes, receiveStruct);

  REQUIRE(sendStruct == receiveStruct);
}
#endif // CONSTEXPR_STRING_AND_VECTOR
