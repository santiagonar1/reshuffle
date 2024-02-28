#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>

#include "utils.hpp"

namespace reshuffle {
    template<typename T>
    auto shuffle(const std::vector<T> &values, const MPI_Comm &comm) {
        int num_ranks{};
        MPI_Comm_size(comm, &num_ranks);

        int total_num_values{static_cast<int>(values.size())};
        MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);
        const int values_per_rank = total_num_values / num_ranks;

        std::vector<T> my_values(values_per_rank);
        MPI_Scatter(values.data(), values_per_rank, details::to_mpi_datatype<T>(), my_values.data(), my_values.size(),
                    details::to_mpi_datatype<T>(),
                    0, comm);

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
