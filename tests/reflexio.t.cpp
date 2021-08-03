#include <catch2/catch.hpp>


#include <app_utils/relfexio.hpp>


REFLEXIO_STRUCT_DEFINE(MyStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc"););

REFLEXIO_STRUCT_DEFINE(MyOtherStruct, 
  REFLEXIO_MEMBER_VAR_DEFINE(int, var1, 12, "var1 doc");
  REFLEXIO_MEMBER_VAR_DEFINE(std::string, var2, "var2_val", "var2 doc"););


TEST_CASE("reflexio_declare", "[reflexio]") { 
  
  MyStruct myStruct; 
  
  REQUIRE(MyStruct::num_members() == 2);

  MyStruct myStruct2;

  REQUIRE(MyStruct::num_members() == 2);

  MyOtherStruct myOtherStruct;

  REQUIRE(MyStruct::num_members() == 2);
}
