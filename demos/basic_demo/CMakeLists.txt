    add_executable(basic_demo.out demo.cpp)

    target_link_libraries(basic_demo.out
            PRIVATE reshuffle
            PRIVATE MPI::MPI_CXX
    )