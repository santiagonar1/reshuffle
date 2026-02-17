    add_executable(scalapack_demo.out scalapack_demo.cpp)
    target_link_libraries(scalapack_demo.out
            PRIVATE MPI::MPI_CXX
            PRIVATE ScalapackLibrary
    )