Include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG        v3.1.0)

FetchContent_MakeAvailable(Catch2)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/contrib) # for finding catch_discover_tests
include(CTest)
include(Catch)

set(CTEST_OUTPUT_ON_FAILURE 1)


file(GLOB_RECURSE TEST_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
file(GLOB_RECURSE TEST_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)


# Tests need to be added as executables first
add_executable(app_utils_tests ${TEST_HEADER_FILES} ${TEST_SOURCE_FILES})

## I'm using C++17 in the test
target_compile_features(app_utils_tests PRIVATE cxx_std_20)


# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(app_utils_tests PRIVATE app_utils Catch2::Catch2 Catch2::Catch2WithMain)


# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
#add_test(NAME app_lib_test COMMAND testlib) # Command can be a target
catch_discover_tests(app_utils_tests)
