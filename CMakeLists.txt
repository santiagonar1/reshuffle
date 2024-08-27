cmake_minimum_required(VERSION 3.26)
project(reshuffle VERSION 0.0.1)

set(CMAKE_CXX_STANDARD 20)
set(namespace "${PROJECT_NAME}")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(MPI REQUIRED)

message(STATUS "Found MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}")

option(BUILD_DYN_PROC_DEMOS "Turn on compilation Dyn Proc Demos. Requires special OpenMPI" OFF)
option(BUILD_TESTING "Build tests" ON)

enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

file(GLOB MY_PUBLIC_HEADERS
        include/*.hpp
)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        MPI::MPI_CXX)

# See: https://decovar.dev/blog/2021/03/08/cmake-cpp-library/#include-directories
target_include_directories(${PROJECT_NAME}
        PRIVATE
        # where the library itself will look for its internal headers
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        PUBLIC
        # where top-level project will look for the library's public headers
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        # where external projects will look for the library's public headers
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

add_subdirectory(demos)

if (BUILD_TESTING)
    add_subdirectory(tests)
    add_subdirectory(benchmarks)
else ()
    message(STATUS "Tests and benchmarks will not be compiled")
endif ()

# where to find our CMake modules
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(Install)