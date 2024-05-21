add_executable(demo.out demo.cpp)
target_link_libraries(demo.out
        PRIVATE reshuffle
        PRIVATE MPI::MPI_CXX)

add_executable(elastic_demo.out elastic_demo.cpp)
target_link_libraries(elastic_demo.out
        PRIVATE reshuffle
        PRIVATE MPI::MPI_CXX)

add_executable(dyn_proc_demo.out dyn_proc_demo.cpp)
target_link_libraries(dyn_proc_demo.out
        PRIVATE reshuffle
        PRIVATE MPI::MPI_CXX)
