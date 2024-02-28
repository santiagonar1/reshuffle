#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>
#include <numeric>

#include "utils.hpp"

namespace reshuffle {
    namespace internal {
        auto calc_num_values_per_rank(int total_num_values, int num_ranks) {
            const int min_num_values_per_rank = total_num_values / num_ranks;
            std::vector<int> values_per_rank(num_ranks, min_num_values_per_rank);
            values_per_rank.back() += total_num_values % num_ranks;

            return values_per_rank;
        }

        auto calc_displacements(const std::vector<int> &num_values_per_rank) {
            std::vector<int> displacements(num_values_per_rank.size(), 0);
            std::partial_sum(num_values_per_rank.begin(), num_values_per_rank.end() - 1, displacements.begin() + 1);

            return displacements;
        }
    }

    template<typename T>
    auto shuffle(const std::vector<T> &values, const MPI_Comm &comm) {
        int num_ranks{};
        int rank{};

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int total_num_values{static_cast<int>(values.size())};
        MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);

        auto counts_send = internal::calc_num_values_per_rank(total_num_values, num_ranks);
        auto displacements = internal::calc_displacements(counts_send);
        const int num_values = counts_send[rank];

        std::vector<T> my_values(num_values);
        MPI_Scatterv(values.data(), counts_send.data(), displacements.data(), internal::to_mpi_datatype<T>(),
                     my_values.data(), num_values, internal::to_mpi_datatype<T>(), 0, comm);

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
