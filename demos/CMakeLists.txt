function(compile_demo demo_name)
    add_executable(${demo_name}.out ${demo_name}.cpp)
    target_link_libraries(${demo_name}.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX)
endfunction()

if (PROFILING)
    compile_demo(profiling)
endif ()

if (TEST_SCALAPACK)
    add_executable(scalapack_demo.out scalapack_demo.cpp)
    target_link_libraries(scalapack_demo.out
            PRIVATE MPI::MPI_CXX
            PRIVATE ScalapackLibrary
    )
endif ()

add_subdirectory(basic_demo)
add_subdirectory(heat_transfer)
add_subdirectory(elastic_demo)

if (BUILD_DYN_PROC_DEMOS)
    add_subdirectory(dyn_proc_demo)
endif ()
