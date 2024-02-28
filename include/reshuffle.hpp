#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>

#include "utils.hpp"

namespace reshuffle {
    template<typename T>
    auto shuffle(const std::vector<T> &values, const MPI_Comm &comm) {
        int num_ranks{};
        int rank{};

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int total_num_values{static_cast<int>(values.size())};
        MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);

        const int min_num_values_per_rank = total_num_values / num_ranks;
        std::vector<int> counts_send(num_ranks, min_num_values_per_rank);
        counts_send.back() += total_num_values % num_ranks;

        std::vector<int> displacements(num_ranks);
        for (int i = 0; i < num_ranks; ++i) {
            displacements[i] = i * min_num_values_per_rank;
        }

        const int num_values = counts_send[rank];

        std::vector<T> my_values(num_values);
        MPI_Scatterv(values.data(), counts_send.data(), displacements.data(), details::to_mpi_datatype<T>(),
                     my_values.data(), num_values, details::to_mpi_datatype<T>(), 0, comm);

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
