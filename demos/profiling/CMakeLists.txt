    add_executable(profiling.out profiling.cpp)

    target_link_libraries(profiling.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX
    )