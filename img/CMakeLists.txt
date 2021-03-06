project(img LANGUAGES CXX)

# for CLion to recognize as project files
file(GLOB IMG_HEADERS
        include/img/*.h)

add_library(img
        src/median.cpp
        src/eval.cpp
        src/fileio.cpp
        ${IMG_HEADERS} src/sgm.cpp)

target_include_directories(img
        PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
            $<INSTALL_INTERFACE:include>
        PRIVATE
            src)

# test executables
add_executable(testfileio
        test/testfileio.cpp)

target_link_libraries(testfileio
        img)

add_executable(testsgm
        test/testsgm.cpp)

target_link_libraries(testsgm
        img)

add_executable(testdirections
        test/testdirections.cpp)

target_link_libraries(testdirections
        img)

add_executable(testeval
        test/testeval.cpp)

target_link_libraries(testeval
        img)

add_executable(testmedian
        test/testmedian.cpp)

target_link_libraries(testmedian
        img)

add_executable(compare_directions
        test/compare_directions.cpp)

target_link_libraries(compare_directions
        img)

add_executable(compare_cost_functions
        test/compare_cost_functions.cpp)

target_link_libraries(compare_cost_functions
        img)

add_executable(test_cost_functions
        test/test_cost_functions.cpp)

target_link_libraries(test_cost_functions
        img)
