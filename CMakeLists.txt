cmake_minimum_required(VERSION 3.8)
project(SGM)

# C++ standard
set(CMAKE_CXX_STANDARD 17)

# warning flags
if ( CMAKE_COMPILER_IS_GNUCC )
    add_compile_options(
            -Wall -Wextra
            -Wduplicated-cond -Wlogical-op -Wrestrict
            -Wnull-dereference -Wold-style-cast -Wuseless-cast
            -Wdouble-promotion -Wshadow -Wformat=2
            -Wswitch -Wswitch-enum
            -Woverloaded-virtual
            -Wundef
            -Wconversion -Wsign-conversion
            -Wduplicated-branches
            -Werror
    )

endif ( CMAKE_COMPILER_IS_GNUCC )
if ( MSVC )
    add_compile_options(
            /w4061 /w4062
            /w4263 /w4264 /w4266
            /w4296 /w4545 /w4546 /w4547 /w4548 /w4549
            /w4574 /4668
            /W4
            /Wx
    )
endif ( MSVC )

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_USE_RESPONSE_FILE_FOR_INCLUDES 1)

# add developed targets
add_subdirectory(app)
add_subdirectory(img)

