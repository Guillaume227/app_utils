Include(FetchContent)

FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/Guillaume227/pybind11
    GIT_TAG        origin/master
)

FetchContent_GetProperties(pybind11)
if(NOT pybind11_POPULATED)
    FetchContent_Populate(pybind11)
    add_subdirectory(${pybind11_SOURCE_DIR} ${pybind11_BINARY_DIR})
endif()


pybind11_add_module(app_utils_test_pybind SHARED
	pybind.t.cpp
)

set(PYTHON_MODULE_NAME app_utils_test)
target_compile_definitions(app_utils_test_pybind PRIVATE REFLEXIO_STRUCT_USE_PYBIND_MODULE=${PYTHON_MODULE_NAME})
target_link_libraries(app_utils_test_pybind PRIVATE app_utils)

# makes the shared library name nice
set_target_properties(app_utils_test_pybind PROPERTIES OUTPUT_NAME ${PYTHON_MODULE_NAME})


# ensures the produced shared library ends up in the project tree
# so it's convenient to fire up a python interpreter and iterate over the local python scripts

set(APP_UTILS_PYTHON_DIR ${CMAKE_CURRENT_LIST_DIR})

if(WIN32)
  set_target_properties(app_utils_test_pybind PROPERTIES SUFFIX ".pyd")

  set_target_properties(app_utils_test_pybind PROPERTIES
		   RUNTIME_OUTPUT_DIRECTORY ${APP_UTILS_PYTHON_DIR})

  set_target_properties(app_utils_test_pybind PROPERTIES
		   RUNTIME_OUTPUT_DIRECTORY_DEBUG ${APP_UTILS_PYTHON_DIR})

  set_target_properties(app_utils_test_pybind PROPERTIES
		   RUNTIME_OUTPUT_DIRECTORY_RELEASE ${APP_UTILS_PYTHON_DIR})

else()
  set_target_properties(app_utils_test_pybind PROPERTIES SUFFIX ".so")

  set_target_properties(app_utils_test_pybind PROPERTIES
		   LIBRARY_OUTPUT_DIRECTORY ${APP_UTILS_PYTHON_DIR})

  set_target_properties(app_utils_test_pybind PROPERTIES
		   LIBRARY_OUTPUT_DIRECTORY_DEBUG ${APP_UTILS_PYTHON_DIR})

  set_target_properties(app_utils_test_pybind PROPERTIES
		   LIBRARY_OUTPUT_DIRECTORY_RELEASE ${APP_UTILS_PYTHON_DIR})

endif()


