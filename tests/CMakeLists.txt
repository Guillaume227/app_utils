
# Testing library
FetchContent_Declare(
  catch
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v2.13.6)
FetchContent_MakeAvailable(catch)
# Adds Catch2::Catch2

file(GLOB_RECURSE TEST_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/*.hpp)
file(GLOB_RECURSE TEST_SOURCE_FILES ${CMAKE_CURRENT_LIST_DIR}/*.cpp)


# Tests need to be added as executables first
add_executable(testlib ${TEST_HEADER_FILES} ${TEST_SOURCE_FILES} )

# I'm using C++17 in the test
target_compile_features(testlib PRIVATE cxx_std_17)


# Should be linked to the main library, as well as the Catch2 testing library
target_link_libraries(testlib PRIVATE app_utils Catch2::Catch2)


# If you register a test, then ctest and make test will run it.
# You can also run examples and check the output, as well.
add_test(NAME app_lib_test COMMAND testlib) # Command can be a target
