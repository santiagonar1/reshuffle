cmake_minimum_required(VERSION 3.27)
project(reshuffle)

set(CMAKE_CXX_STANDARD 23)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

find_package(MPI REQUIRED)
message(STATUS "Found MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}")

enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

add_library(reshuffle
        ${SOURCES})

target_include_directories(reshuffle
        PUBLIC "${reshuffle_SOURCE_DIR}/include"
        PUBLIC MPI::MPI_CXX)

add_subdirectory(apps)
add_subdirectory(tests)
