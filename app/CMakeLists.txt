# define executable
add_executable(SGM
        src/main.cpp)

target_link_libraries(SGM
        PRIVATE
            img)

target_include_directories(SGM
        PRIVATE
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        )

