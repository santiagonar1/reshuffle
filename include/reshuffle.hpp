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

        template<typename T>
        auto get_all_values(const std::vector<T> &values, const MPI_Comm &comm) {
            int num_ranks{};
            int rank{};
            MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

            MPI_Comm_size(comm, &num_ranks);
            MPI_Comm_rank(comm, &rank);

            int num_values{static_cast<int>(values.size())};
            std::vector<int> num_values_per_rank(num_ranks);
            MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
            int total_num_values = std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

            auto all_values = rank == 0 ? std::vector<T>(total_num_values) : std::vector<T>{};
            auto displacements = calc_displacements(num_values_per_rank);
            MPI_Gatherv(values.data(), num_values, mpi_datatype, all_values.data(), num_values_per_rank.data(),
                        displacements.data(), mpi_datatype, 0, comm);

            return all_values;
        }

        template<typename T>
        auto scatter_values(const std::vector<T> &values, const MPI_Comm &comm) {
            int num_ranks{};
            int rank{};
            MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

            MPI_Comm_size(comm, &num_ranks);
            MPI_Comm_rank(comm, &rank);

            int total_num_values = static_cast<int>(values.size());
            MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);

            auto new_num_values_per_rank = internal::calc_num_values_per_rank(total_num_values, num_ranks);
            const auto displacements = internal::calc_displacements(new_num_values_per_rank);
            const int new_num_values = new_num_values_per_rank[rank];

            std::vector<T> my_values(new_num_values);
            MPI_Scatterv(values.data(), new_num_values_per_rank.data(), displacements.data(), mpi_datatype,
                         my_values.data(),
                         new_num_values, mpi_datatype, 0, comm);

            return my_values;
        }
    }

    template<typename T>
    auto shuffle(const std::vector<T> &values, const MPI_Comm &comm) {
        const auto all_values = internal::get_all_values(values, comm);
        const auto my_values = internal::scatter_values(all_values, comm);

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
