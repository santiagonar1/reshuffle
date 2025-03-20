find_package(benchmark REQUIRED)

function(compile_benchmark benchmark_name)
    add_executable(${benchmark_name}.out ${benchmark_name}.cpp)
    target_link_libraries(${benchmark_name}.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX
            PRIVATE benchmark::benchmark)
endfunction()

compile_benchmark(buffer_benchmarks)
