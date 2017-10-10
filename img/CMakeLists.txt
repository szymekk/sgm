project(img LANGUAGES CXX)

add_library(img
        src/fileio.cpp)

target_include_directories(img
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            src)

