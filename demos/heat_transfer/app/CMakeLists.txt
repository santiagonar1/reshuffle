set(EXECUTABLE_NAME ${LIBRARY_NAME}.out)

find_package(absl REQUIRED)

add_executable(${EXECUTABLE_NAME} main.cpp)

target_link_libraries(${EXECUTABLE_NAME}
        PRIVATE ${LIBRARY_NAME}
        PRIVATE reshuffle
        PRIVATE MPI::MPI_CXX
        PRIVATE abseil::abseil
)