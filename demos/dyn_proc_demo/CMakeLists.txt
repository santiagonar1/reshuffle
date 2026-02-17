    find_package(absl REQUIRED)

    add_executable(dyn_proc_demo.out dyn_proc_demo.cpp)
    target_link_libraries(dyn_proc_demo.out
            PRIVATE abseil::abseil
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX
    )