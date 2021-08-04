#include <catch2/catch.hpp>

#include <app_utils/relfexio.hpp>


REFLEXIO_STRUCT_DEFINE(MyStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc"););

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc"););


TEST_CASE("reflexio_declare", "[reflexio]") { 

  REQUIRE(MyStruct::num_members() == 2);

  MyStruct myStruct; 
  
  REQUIRE(MyStruct::num_members() == 2);

  MyStruct myStruct2;

  REQUIRE(MyStruct::num_members() == 2);

  MyOtherStruct myOtherStruct;

  REQUIRE(MyStruct::num_members() == 2);


  REQUIRE(MyStruct::get_member_descriptors()[0]->default_value_as_string() == "12");
  REQUIRE(MyStruct::get_member_descriptors()[1]->default_value_as_string() == "var2_val");

  REQUIRE(myStruct == myStruct2);
  REQUIRE(myStruct.has_all_default_values());
  myStruct.var1 = 11;
  REQUIRE(not myStruct.has_all_default_values());
  REQUIRE(myStruct != myStruct2);
}
