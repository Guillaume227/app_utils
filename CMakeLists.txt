cmake_minimum_required(VERSION 3.15)

project(app_utils
 VERSION 0.1
 DESCRIPTION "collection of c++ utilities" LANGUAGES CXX)

 # Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  include(cmake/main_project_utils.cmake)
  include(cmake/cpp_utils.cmake)
  include(cmake/git_utils.cmake)
endif()

file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/include/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)

add_library(app_utils STATIC ${HEADER_FILES} ${SOURCE_FILES})

set_target_properties(app_utils PROPERTIES CXX_STANDARD 20)

target_include_directories(app_utils PUBLIC ./include)

# All users of this library will need at least C++20
target_compile_features(app_utils PUBLIC cxx_std_20)


# Testing only available if this is the main app
# Emergency override MODERN_CMAKE_BUILD_TESTING provided as well
if((CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR MODERN_CMAKE_BUILD_TESTING)
   AND BUILD_TESTING)
  add_subdirectory(tests)
  add_subdirectory(tests_py)
endif()

if (UNIX)
	set(CPP_WARN -fsingle-precision-constant
				 -Wdouble-promotion
				 -Wall
				 -Wextra
				 -Wundef
				 -Werror
				 -Wfatal-errors)

	target_compile_options(app_utils PUBLIC ${CPP_WARN} -fPIC)
	#set_target_properties(app_utils  PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

endif(UNIX)
