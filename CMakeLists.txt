cmake_minimum_required(VERSION 3.26)
project(reshuffle)

set(CMAKE_CXX_STANDARD 20)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(MPI REQUIRED)
find_package(zpp_bits REQUIRED)
find_package(benchmark REQUIRED)

message(STATUS "Found MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}")

option(BUILD_DYN_PROC_DEMOS "Turn on compilation Dyn Proc Demos. Requires special OpenMPI" OFF)

enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

add_library(reshuffle STATIC)

target_sources(reshuffle
        PRIVATE ${SOURCES})

target_link_libraries(reshuffle
        MPI::MPI_CXX
        zpp_bits::zpp_bits)

target_include_directories(reshuffle
        PUBLIC "${reshuffle_SOURCE_DIR}/include")

add_subdirectory(benchmarks)
add_subdirectory(demos)
add_subdirectory(tests)
