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

file(GLOB MY_PUBLIC_HEADERS
        include/*.hpp
)

add_library(${PROJECT_NAME})

target_sources(${PROJECT_NAME}
        PRIVATE ${SOURCES})

target_link_libraries(${PROJECT_NAME}
        MPI::MPI_CXX
        zpp_bits::zpp_bits)

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

# note that it is not CMAKE_INSTALL_PREFIX we are checking here
if (DEFINED CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(
            STATUS
            "CMAKE_INSTALL_PREFIX is not set\n"
            "Default value: ${CMAKE_INSTALL_PREFIX}\n"
            "Will set it to ${CMAKE_SOURCE_DIR}/install"
    )
    set(CMAKE_INSTALL_PREFIX
            "${CMAKE_SOURCE_DIR}/install"
            CACHE PATH "Where the library will be installed to" FORCE
    )
else ()
    message(
            STATUS
            "CMAKE_INSTALL_PREFIX was already set\n"
            "Current value: ${CMAKE_INSTALL_PREFIX}"
    )
endif ()

# without it public headers won't get installed
set(public_headers
        ${MY_PUBLIC_HEADERS}
)
# note that ${public_headers} has to be in quotes
set_target_properties(${PROJECT_NAME} PROPERTIES PUBLIC_HEADER "${public_headers}")

add_subdirectory(benchmarks)
add_subdirectory(demos)
add_subdirectory(tests)
