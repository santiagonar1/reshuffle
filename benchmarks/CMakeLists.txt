find_package(benchmark REQUIRED)

function(compile_benchmark benchmark_name)
    add_executable(${benchmark_name}.out ${benchmark_name}.cpp)
    target_link_libraries(${benchmark_name}.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX
            PRIVATE benchmark::benchmark
    )
    target_include_directories(${benchmark_name}.out
            PRIVATE include
    )
endfunction()

compile_benchmark(reshuffle_benchmark)
compile_benchmark(strategy_benchmark)
compile_benchmark(reshuffle_reordering_benchmark)

if (TEST_SCALAPACK)
    add_executable(scalapack_benchmark.out scalapack_benchmark.cpp)
    target_link_libraries(scalapack_benchmark.out
            PRIVATE MPI::MPI_CXX
            PRIVATE ScalapackLibrary
            PRIVATE benchmark::benchmark
    )
    target_include_directories(scalapack_benchmark.out
            PRIVATE include
    )
endif ()

if (TEST_COSTA)
    add_executable(costa_benchmark.out costa_benchmark.cpp)
    target_link_libraries(costa_benchmark.out
            PRIVATE MPI::MPI_CXX
            PRIVATE CostaLibrary
            PRIVATE benchmark::benchmark
    )
    target_include_directories(costa_benchmark.out
            PRIVATE include
    )
endif ()