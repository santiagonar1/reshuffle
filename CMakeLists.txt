cmake_minimum_required(VERSION 3.26)
project(reshuffle VERSION 0.0.1)

# Check for minimum GCC version
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 14)
        message(FATAL_ERROR "GCC version must be at least 14. Detected version is ${CMAKE_CXX_COMPILER_VERSION}")
    endif ()
endif ()

set(namespace "${PROJECT_NAME}")

# Where to find our CMake modules. Needs to be called before any add_subdirectory for them to have access to the variable
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif ()

find_package(MPI REQUIRED)
find_package(zpp_bits REQUIRED)

message(STATUS "Found MPI_CXX_COMPILER: ${MPI_CXX_COMPILER}")

option(BUILD_DYN_PROC_DEMOS "Turn on compilation Dyn Proc Demos. Requires special OpenMPI" OFF)
option(BUILD_TESTING "Build tests" ON)
option(BUILD_DEMOS "Build demos" ON)
option(TEST_SCALAPACK "Build examples and benchmarks of ScaLAPACK" OFF)
option(TEST_COSTA "Build examples and benchmarks of the COSTA library" OFF)
option(PROFILING "Turns profiling on" OFF)

if (TEST_SCALAPACK)
    include(FindScalapackPackages)
endif ()

if (TEST_COSTA)
    include(FindCostaPackages)
endif ()

if (PROFILING)
    message(STATUS "Profiling is enabled - Tracy profiler will be used")
    find_package(Tracy REQUIRED)
endif ()


enable_testing()

file(GLOB SOURCES
        src/*.cpp
)

file(GLOB MY_PUBLIC_HEADERS
        include/*.hpp
)

add_library(${PROJECT_NAME})
target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_sources(${PROJECT_NAME}
        PRIVATE ${SOURCES}
)

target_link_libraries(${PROJECT_NAME}
        PUBLIC MPI::MPI_CXX
        PUBLIC zpp_bits::zpp_bits
)

if (PROFILING)
    target_compile_definitions(${PROJECT_NAME} PUBLIC ENABLE_PROFILING_RESHUFFLE)
    target_link_libraries(${PROJECT_NAME}
            PUBLIC Tracy::TracyClient)
endif ()

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

include(Install)