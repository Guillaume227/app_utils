cmake_minimum_required(VERSION 3.10)

project(app_utils)


file(GLOB_RECURSE HEADER_FILES ${PROJECT_SOURCE_DIR}/*.hpp)
file(GLOB_RECURSE SOURCE_FILES ${PROJECT_SOURCE_DIR}/*.cpp)

add_library(app_utils STATIC ${HEADER_FILES} ${SOURCE_FILES} )

set_target_properties(app_utils PROPERTIES CXX_STANDARD 17)

target_include_directories(app_utils PUBLIC ${PROJECT_SOURCE_DIR})


if (UNIX)
	set(CPP_WARN -fsingle-precision-constant
				 -Wdouble-promotion
				 -Wall
				 -Wextra
				 -Wundef
				 -Werror)

	target_compile_options(app_utils PUBLIC ${CPP_WARN})
	set_target_properties(app_utils  PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

endif(UNIX)
