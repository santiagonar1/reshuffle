function (compile_demo demo_name)
    add_executable(${demo_name}.out ${demo_name}.cpp)
    target_link_libraries(${demo_name}.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX)
endfunction()

compile_demo(demo)
compile_demo(elastic_demo)

if (BUILD_DYN_PROC_DEMOS)
    find_package(absl REQUIRED)

    add_executable(dyn_proc_demo.out dyn_proc_demo.cpp)
    target_link_libraries(dyn_proc_demo.out
            PRIVATE abseil::abseil
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX)
endif ()