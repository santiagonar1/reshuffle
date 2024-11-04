#include "mpi_utils.hpp"

namespace reshuffle::internal {
    auto get_rank_id(const MPI_Comm &comm) -> rank_id {
        int rank{MPI_ERR_RANK};

        if (is_comm_null(comm)) {
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

        if (is_comm_null(comm)) {
            throw std::invalid_argument("Invalid MPI_COMM_NULL communicator");
        }

        MPI_Comm_size(comm, &num_ranks);
        return num_ranks;
    }

    auto is_comm_null(const MPI_Comm &comm) -> bool { return comm == MPI_COMM_NULL; }


    auto get_displacements(const std::vector<int> &num_values_per_rank) -> std::vector<int> {
        std::vector displacements(num_values_per_rank.size(), 0);
        std::partial_sum(num_values_per_rank.begin(), num_values_per_rank.end() - 1,
                         displacements.begin() + 1);

        return displacements;
    }

}// namespace reshuffle::internal
