cmake_minimum_required(VERSION 3.27)
project(reshuffle)

set(CMAKE_CXX_STANDARD 23)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(MPI REQUIRED)
find_package(zpp_bits REQUIRED)

message(STATUS "Found MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}")

enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

add_library(reshuffle
        ${SOURCES})

target_link_libraries(reshuffle
        MPI::MPI_CXX
        zpp_bits::zpp_bits)

target_include_directories(reshuffle
        PUBLIC "${reshuffle_SOURCE_DIR}/include")

add_subdirectory(apps)
add_subdirectory(tests)
