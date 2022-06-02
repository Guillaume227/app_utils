
execute_process(COMMAND git log --pretty=format:'%h' -n 1
                OUTPUT_VARIABLE GIT_REV
                ERROR_QUIET)

# Check whether we got any revision (which isn't
# always the case, e.g. when someone downloaded a zip
# file from Github instead of a checkout)
if ("${GIT_REV}" STREQUAL "")
    set(GIT_REV "N/A")
    set(GIT_DIFF "")
    set(GIT_DIFF_NUM "0")
    set(GIT_TAG "N/A")
    set(GIT_BRANCH "N/A")
else()
    execute_process(
        COMMAND bash -c "git diff --quiet --exit-code || echo +"
        OUTPUT_VARIABLE GIT_DIFF)
    execute_process(
        COMMAND bash -c "git diff --quiet --exit-code || echo F"
        OUTPUT_VARIABLE GIT_DIFF_NUM)
    execute_process(
        COMMAND git describe --exact-match --tags
        OUTPUT_VARIABLE GIT_TAG ERROR_QUIET)
    execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        OUTPUT_VARIABLE GIT_BRANCH)

    string(STRIP "${GIT_REV}" GIT_REV)
    string(SUBSTRING "${GIT_REV}" 1 7 GIT_REV)
    string(STRIP "${GIT_DIFF}" GIT_DIFF)
    string(STRIP "${GIT_DIFF_NUM}" GIT_DIFF_NUM)
    string(STRIP "${GIT_TAG}" GIT_TAG)
    string(STRIP "${GIT_BRANCH}" GIT_BRANCH)
endif()

set(VERSION 
"
/* AUTOMATICALLY GENERATED - DO NOT COMMIT, DO NOT EDIT AS IT WILL BE OVERWRITTEN */
#include \"version.h\"

namespace version {
char const* git_rev() { return \"${GIT_REV}${GIT_DIFF}\"; }
char const* git_tag() { return \"${GIT_TAG}\"; }
char const* git_branch() { return \"${GIT_BRANCH}\"; }
char const* compilation_timestamp() { return __DATE__ \":\" __TIME__; }
}"
)

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp VERSION_)
else()
    set(VERSION_ "")
endif()

if (NOT "${VERSION}" STREQUAL "${VERSION_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.cpp "${VERSION}")
endif()


set(VERSION_H
        "
/* AUTOMATICALLY GENERATED - DO NOT COMMIT, DO NOT EDIT AS IT WILL BE OVERWRITTEN */
#pragma once
#include <stdint.h>
#define VERSION_APP 0x${GIT_REV}${GIT_DIFF_NUM}

namespace version {
char const* git_rev();
char const* git_tag();
char const* git_branch();
char const* compilation_timestamp();
}
")

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/version.h)
    file(READ ${CMAKE_CURRENT_SOURCE_DIR}/version.h VERSION_H_)
else()
    set(VERSION_H_ "")
endif()

if (NOT "${VERSION_H}" STREQUAL "${VERSION_H_}")
    file(WRITE ${CMAKE_CURRENT_SOURCE_DIR}/version.h "${VERSION_H}")
endif()