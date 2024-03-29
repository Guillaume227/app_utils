cmake_minimum_required(VERSION 3.15)

#include this module under:
#[[
# Only do these if this is the main project, and not if it is included through add_subdirectory
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
]]

# Let's nicely support folders in IDEs
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# this line helps hide annoying automatically generated CTest targets
# that clutter IDEs like Clion
# see: https://gitlab.kitware.com/cmake/cmake/-/issues/21730
set_property(GLOBAL PROPERTY CTEST_TARGETS_ADDED 1)

# Testing only available if this is the main app
# Note this needs to be done in the main CMakeLists
# since it calls enable_testing, which must be in the
# main CMakeLists.
include(CTest)

# Docs only available if this is the main app
find_package(Doxygen)
if(Doxygen_FOUND)
  if(EXISTS docs)
  	add_subdirectory(docs)
  endif()
else()
  message(STATUS "Doxygen not found, not building docs")
endif()

include(FetchContent)
