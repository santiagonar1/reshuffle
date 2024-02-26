#include <iostream>
#include <reshuffle.hpp>

#include <mpi.h>

int main() {
    int rank{};

    MPI_Init(nullptr, nullptr);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    std::cout << "Hello, from rank " << rank << std::endl;

    MPI_Finalize();
    return 0;
}
