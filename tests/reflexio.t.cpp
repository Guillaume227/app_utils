#include <catch2/catch.hpp>

#include <app_utils/relfexio.hpp>

#include <app_utils/enumatic.hpp>
#include <app_utils/cond_check.hpp>

ENUMATIC_DEFINE(TestEnum, EnumVal1, EnumVal2);

REFLEXIO_STRUCT_DEFINE(MyStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(float, var3, 1.5f, "var3 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(TestEnum, var4, TestEnum::EnumVal2, "var4 doc");
  );

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc"););

TEST_CASE("reflexio_declare", "[reflexio]") { 

  REQUIRE(MyStruct::num_members() == 4);

  MyStruct myStruct; 
  
  REQUIRE(MyStruct::num_members() == 4);

  MyStruct myStruct2;

  REQUIRE(MyStruct::num_members() == 4);

  MyOtherStruct myOtherStruct;

  REQUIRE(MyStruct::num_members() == 4);

  REQUIRE(myStruct == myStruct2);
  REQUIRE(myStruct.has_all_default_values());
  myStruct.var1 = 11;
  REQUIRE(not myStruct.has_all_default_values());
  REQUIRE(myStruct != myStruct2);

  REQUIRE(MyStruct::get_member_descriptors()[0]->default_value_as_string() == "12");
  REQUIRE(MyStruct::get_member_descriptors()[1]->default_value_as_string() == "var2_val");
  //REQUIRE(MyStruct::get_member_descriptors()[2]->default_value_as_string() == "1.5");
  REQUIRE(MyStruct::get_member_descriptors()[3]->default_value_as_string() == "EnumVal2");
}

TEST_CASE("reflexio_serialize", "[reflexio]") {

  MyStruct sendStruct;
  sendStruct.var1 = 2;
  sendStruct.var2 = "new_val";
  sendStruct.var3 = 0.5;
  sendStruct.var4 = TestEnum::EnumVal1;

  MyStruct receiveStruct;
  REQUIRE(sendStruct != receiveStruct);
  std::vector<std::byte> buffer(256);
  
  size_t written_bytes = to_bytes(buffer.data(), buffer.size(), sendStruct);

  from_bytes(buffer.data(), written_bytes, receiveStruct);

  REQUIRE(sendStruct == receiveStruct);
}