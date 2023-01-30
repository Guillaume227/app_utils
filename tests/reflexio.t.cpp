#include <catch2/catch_test_macros.hpp>

#include "reflexio.t.hpp"

#include <app_utils/cond_check.hpp>
#include <app_utils/serial_type_utils.hpp>
#include <app_utils/serial_utils.hpp>
#include <app_utils/log_utils.hpp>

TEST_CASE("reflexio_single_var_struct", "[reflexio]") {
  SingleVarStruct singleVarStruct;
  REQUIRE(singleVarStruct.has_all_default_values());
  REQUIRE(to_yaml(singleVarStruct) == "var1: " + std::to_string(singleVarStruct.var1) + "\n");
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
  REQUIRE(mask_var1 == TrivialStruct::Mask{2});
  auto mask_var2 = TrivialStruct::make_vars_mask(&TrivialStruct::var2);
  REQUIRE(mask_var2 == TrivialStruct::Mask{1});

  auto mask_var_all = TrivialStruct::make_vars_mask(&TrivialStruct::var1,
                                                    &TrivialStruct::var2);
  REQUIRE(mask_var_all == TrivialStruct::Mask{});
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

  //REQUIRE(MyStruct::get_member_descriptors()[0]->default_to_yaml() == "12");
  //REQUIRE(MyStruct::get_member_descriptors()[1]->default_to_yaml() == "1.500000");
  //REQUIRE(MyStruct::get_member_descriptors()[2]->default_to_yaml() == "EnumVal2");
  //REQUIRE(MyStruct::get_member_descriptors()[3]->default_to_yaml() == "true"); // TODO: should be "true"
}

TEST_CASE("reflexio_serialize", "[reflexio]") {

  TrivialStruct trivialStruct;
  REQUIRE(trivialStruct.get_serial_size() == sizeof(float) + sizeof(int8_t));

  REQUIRE(serial_size(trivialStruct) == sizeof(float) + sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::Mask{0}) == sizeof(float) + sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::Mask{1}) == sizeof(float));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::Mask{2}) == sizeof(int8_t));
  REQUIRE(serial_size(trivialStruct, TrivialStruct::Mask{3}) == 0);

  MyStruct sendStruct;
  sendStruct.var1 = 2;
  sendStruct.var2 = 0.5;
  sendStruct.var3 = TestEnum::EnumVal1;
  sendStruct.var4 = false;
  sendStruct.var5 = {1.f, 2.f, 3.f, 6.f, 5.f, 4.f, 7.f, 8.f};
  {
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

  { // Partial serialization of a subset of the members
    auto vars_mask = MyStruct::make_vars_mask(&MyStruct::var2, &MyStruct::var5);
    std::vector<std::byte> buffer(256);
    size_t const written_bytes = to_bytes(buffer, sendStruct, vars_mask);
    buffer.resize(written_bytes);

    MyStruct receiveStruct;
    from_bytes(buffer, receiveStruct, vars_mask);

    MyStruct expectedStruct;
    expectedStruct.var2 = sendStruct.var2;
    expectedStruct.var5 = sendStruct.var5;

    REQUIRE(expectedStruct != sendStruct);
    REQUIRE(expectedStruct == receiveStruct);
  }
}

TEST_CASE("reflexio_constexpr", "[reflexio]") {
  constexpr TrivialStruct myStruct_const;

  constexpr size_t myStruct_size = myStruct_const.get_serial_size();
  REQUIRE(myStruct_size == sizeof(float) + sizeof(int8_t));

  constexpr TrivialStruct::Mask filter{1};
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
  {
    std::ostringstream oss;
    SingleVarStruct SingleVarStruct;
    for (auto& descriptor: SingleVarStruct) {
      oss << descriptor.get_name() << ";";
    }
    REQUIRE(oss.str() == "var1;");
  }
  {
    MyStruct myStruct;
    size_t i = 0;
    for (auto& descriptor: myStruct) {
      LOG_LINE(i++, descriptor.get_name());
    }
    REQUIRE(i == MyStruct::num_registered_member_vars());
  }
}

TEST_CASE("reflexio_get_value", "[reflexio]") {
  MyStruct myStruct;
  auto& descriptors = myStruct.get_member_descriptors();
  REQUIRE(descriptors[1]->get_value_ref<float>(&myStruct) == myStruct.var2);
  REQUIRE(descriptors[2]->get_value_ref<TestEnum>(&myStruct) == myStruct.var3);
  std::span values_list = descriptors[2]->get_values_str();

  REQUIRE(values_list.size() > 0);
  std::span values = Enumatic<TestEnum>::get_values_str();
  REQUIRE(values_list.size() == values.size());
}

TEST_CASE("reflexio_from_string", "[reflexio]") {

  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(var1: 11 # some comment at end of line
    # Some comment line
    var2: 22.22)";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    refStruct.var2 = 22.22f;
    REQUIRE(reflexioStruct == refStruct);

    std::string valstr = to_yaml(reflexioStruct);
    reflexioStruct.var2 = 33.33f;
    from_yaml(reflexioStruct, valstr);

    REQUIRE(reflexioStruct == refStruct);
  }
  { // Selective deserialization
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(var1: 11
    var2: 22.22)";
    auto mask = TrivialStruct::make_vars_mask(&TrivialStruct::var1);
    from_yaml(reflexioStruct, val_str, mask);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    REQUIRE(reflexioStruct == refStruct);

    {
      TrivialStruct::FatView view;
      from_yaml(view, val_str);
      REQUIRE(view.object != refStruct);
    }
    {
      TrivialStruct::FatView view(mask);
      from_yaml(view, val_str);
      REQUIRE(view.object == refStruct);
    }
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(   var1: 11      )";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = "var1: 11\nvar2: 22";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    refStruct.var2 = 22;
    REQUIRE(reflexioStruct == refStruct);
  }
  {// with no extra new lines
    TrivialStruct reflexioStruct;
    std::string_view val_str = "var1: 11 # some comment";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var1 = 11;
    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = R"(var2: 22.22)";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    refStruct.var2 = 22.22f;
    REQUIRE(reflexioStruct == refStruct);
  }
  {
    TrivialStruct reflexioStruct;
    std::string_view val_str = "";
    from_yaml(reflexioStruct, val_str);
    TrivialStruct refStruct;
    REQUIRE(reflexioStruct == refStruct);
  }
}

TEST_CASE("reflexio_composite", "[reflexio]") {

  NestedStruct myStruct;
  REQUIRE(myStruct.has_all_default_values());
  myStruct.struct1.var1 = 2;

  REQUIRE(not myStruct.has_all_default_values());

  auto myStructCopy = myStruct;
  REQUIRE(myStructCopy == myStruct);
  auto& subStruct = myStructCopy.struct1;
  subStruct.var2 = 22.f;

  REQUIRE(myStructCopy != myStruct);

  auto val_str = to_yaml(myStruct);
  std::cout << "---" << std::endl
            << val_str
            << "..." << std::endl;

  from_yaml(myStructCopy, val_str);

  REQUIRE(myStructCopy == myStruct);
}

TEST_CASE("reflexio_yaml_sections", "[reflexio]") {

  for (int i = 0; i <= 1; i++) {
    // tests with and without '...' separator
    // which seems to be optional in yaml.
    bool const with_end_separator = i > 0;

    TrivialStruct myStruct;
    REQUIRE(myStruct.has_all_default_values());
    myStruct.var1 *= 2;
    myStruct.var2 *= 2;

    REQUIRE(not myStruct.has_all_default_values());

    TrivialStruct otherStruct;
    otherStruct.var1 += 1;
    otherStruct.var2 += 1;

    REQUIRE(otherStruct != myStruct);

    std::ostringstream oss;
    oss << "---\n"
        << myStruct;
    if (with_end_separator) {
      oss << "...\n";
    }

    oss << "---\n"
        << otherStruct;

    std::string const val_str = oss.str();
    //std::cout << val_str;
    std::istringstream iss(val_str);

    TrivialStruct myStruct2;
    TrivialStruct otherStruct2;

    REQUIRE(myStruct != myStruct2);
    REQUIRE(otherStruct != otherStruct2);

    from_yaml(myStruct2, iss);
    from_yaml(otherStruct2, iss);

    REQUIRE(myStruct == myStruct2);
    //std::cerr << "differences: " << otherStruct.differences(otherStruct2) << std::endl;
    //REQUIRE(not otherStruct.differs(otherStruct2));
    REQUIRE(otherStruct == otherStruct2);
  }
}

TEST_CASE("reflexio_yaml_io", "[reflexio]") {

    FancierStruct myStruct_in;
    myStruct_in.var1 = 256;
    myStruct_in.var2 = 3.14f;
    myStruct_in.var3 = MyEnum::EnumVal1;
    myStruct_in.var4 = MyOtherEnum::EnumVal3;
    myStruct_in.var5 = false;
    myStruct_in.var6 = {1, 2, 3, 4, 5, 6, 7, 8};
    myStruct_in.var7 = 10.f;
    myStruct_in.var8 = 20.f;

    FancierStruct myStruct_out;

    auto val_str = to_yaml(myStruct_in);
    std::cout << val_str << std::endl;
    from_yaml(myStruct_out, val_str);

    REQUIRE(myStruct_in == myStruct_out);
}

TEST_CASE("reflexio_view_serialization", "[reflexio]") {

  auto excludeMask = FancierStruct::make_vars_mask(&FancierStruct::var2,
                                                   &FancierStruct::var5,
                                                   &FancierStruct::var7);
  FancierStruct::FatView view {excludeMask};
  view.object.var2 = 6.28f;
  view.object.var5 = true;
  view.object.var7 = 10.f;
  view.object.var8 = 40.f;

  std::vector<std::byte> buffer;
  app_utils::serial::to_bytes(buffer, view);

  REQUIRE(buffer.size() < serial_size(view.object));
  REQUIRE(buffer.size() == serial_size(view));

  REQUIRE(serial_size(FancierStruct::View(view.object)) == 2 + serial_size(view.object));

  FancierStruct myStruct2;
  FancierStruct::View view2 {myStruct2, excludeMask};
  app_utils::serial::from_bytes(buffer, view2);


  REQUIRE(view.object != myStruct2);
  myStruct2.var8 = view.object.var8;
  REQUIRE(view.object == myStruct2);
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

  auto const default_as_str = [&](size_t index){
    std::ostringstream os;
    StructWithStringAndVector::get_member_descriptors()[index]->default_to_yaml(os);
    return os.str();
  };

  REQUIRE(default_as_str(0) == "12");
  REQUIRE(default_as_str(1) == "var2_val");
  // transient constexpr allocation: see comment in reflexio.hpp around reflexio_traits::DefaultType specialization
  //REQUIRE(StructWithStringAndVector::get_member_descriptors()[2]->default_to_yaml() == "{10.f}");

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
