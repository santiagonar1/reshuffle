cmake_minimum_required(VERSION 3.27)
project(reshuffle)

set(CMAKE_CXX_STANDARD 23)

enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

add_library(reshuffle
        ${SOURCES})

target_include_directories(reshuffle
        PUBLIC "${reshuffle_SOURCE_DIR}/include")


add_subdirectory(apps)
add_subdirectory(tests)
