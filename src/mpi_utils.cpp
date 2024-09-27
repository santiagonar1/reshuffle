#include "mpi_utils.hpp"

namespace reshuffle::internal {
    auto get_rank_id(const MPI_Comm &comm) -> rank_id {
        int rank{MPI_ERR_RANK};

        if (comm == MPI_COMM_NULL) {
            throw std::invalid_argument("Invalid MPI_COMM_NULL communicator");
        }

        MPI_Comm_rank(comm, &rank);
        return rank;
    }

    auto in_mpi_comm(const MPI_Comm &comm) -> bool { return comm != MPI_COMM_NULL; }

    auto is_root(const MPI_Comm &comm) -> bool {
        return in_mpi_comm(comm) and get_rank_id(comm) == 0;
    }

    auto get_num_ranks(const MPI_Comm &comm) -> int {
        int num_ranks{};
        MPI_Comm_size(comm, &num_ranks);
        return num_ranks;
    }

}// namespace reshuffle::internal
