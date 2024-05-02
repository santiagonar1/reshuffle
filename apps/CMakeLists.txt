add_executable(demo.out demo.cpp)
target_link_libraries(demo.out
        PRIVATE reshuffle
        PRIVATE MPI::MPI_CXX)
