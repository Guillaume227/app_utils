

# Let's ensure -std=c++xx instead of -std=g++xx
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 20)


if (MSVC)

    #trick to suppress annoying warning related to MSVC flags.
    string(REGEX REPLACE "/W[3|4]" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

    # https://stackoverflow.com/questions/24414124/why-does-vs-not-define-the-alternative-tokens-for-logical-operators
    add_compile_options(
            #"/Za"
            #"/Wall"
            /W4
            /Zc:preprocessor # by default MSVC uses a legacy preprocessor
            /WX # treat warnings as errors
            /permissive- # so we can use alternative operators with msvc
            -D_CRT_SECURE_NO_WARNINGS # remove useless std::getenv windows warning
            )

endif ()

if (UNIX)
    set(CPP_WARN -fsingle-precision-constant
            -Wdouble-promotion
            -Wall
            -Wextra
            -Wundef
            -Werror
            -Wfatal-error)

    #target_compile_options(app_utils PUBLIC ${CPP_WARN})
    #set_target_properties(app_utils  PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

endif (UNIX)
