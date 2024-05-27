function (compile_demo demo_name)
    add_executable(${demo_name}.out ${demo_name}.cpp)
    target_link_libraries(${demo_name}.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX)
endfunction()

compile_demo(demo)
compile_demo(elastic_demo)

if (BUILD_DYN_PROC_DEMOS)
    compile_demo(dyn_proc_demo)
endif ()