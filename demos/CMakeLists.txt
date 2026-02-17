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

if (PROFILING)
    add_subdirectory(profiling)
endif ()
