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


pybind11_add_module(app_utils_test_pybind pybind.t.cpp)

#set(PYTHON_MODULE_NAME app_utils_test)
#target_compile_definitions(app_utils_test_pybind PRIVATE REFLEXIO_STRUCT_USE_PYBIND_MODULE=${PYTHON_MODULE_NAME})
target_link_libraries(app_utils_test_pybind PRIVATE app_utils)
target_include_directories(app_utils_test_pybind PRIVATE tests)

# makes the shared library name nice
#set_target_properties(app_utils_test_pybind PROPERTIES OUTPUT_NAME
#        ${PYTHON_MODULE_NAME})

option(APP_UTILS_EXTENSION_LOCAL_COPY
        "Copy python extension shared object locally for easier testing"
        ON)

#if(APP_UTILS_EXTENSION_LOCAL_COPY)
set(PYTHON_PACKAGE_DIR ${CMAKE_CURRENT_LIST_DIR})

# ensures the produced shared library ends up in the project root
# so it's convenient to fire up a python interpreter and
# iterate over the local python scripts
set_target_properties(app_utils_test_pybind PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${PYTHON_PACKAGE_DIR})

set_target_properties(app_utils_test_pybind PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${PYTHON_PACKAGE_DIR})
#endif()

add_test (NAME app_utils-pybind-tests
  COMMAND ${PYTHON_EXECUTABLE} -m pytest ${CMAKE_CURRENT_SOURCE_DIR} -rs --verbose --capture=no
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
)

set_tests_properties(app_utils-pybind-tests
        PROPERTIES DEPENDS app_utils_test_pybind)
