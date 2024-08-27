#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <mpi.h>
#include <numeric>
#include <ranges>
#include <vector>

#include "concepts.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle::internal {
    auto get_rank_id(const MPI_Comm &comm) -> rank_id {
        int rank{};
        MPI_Comm_rank(comm, &rank);
        return rank;
    }

    auto in_mpi_comm(const MPI_Comm &comm) { return comm != MPI_COMM_NULL; }

    auto is_root(const MPI_Comm &comm) { return in_mpi_comm(comm) and get_rank_id(comm) == 0; }

    auto num_ranks(const MPI_Comm &comm) {
        int num_ranks{};
        MPI_Comm_size(comm, &num_ranks);
        return num_ranks;
    }

    auto calc_num_values_per_rank(int num_ranks, const std::vector<rank_id> &coloring) {
        std::vector<int> values_per_rank(num_ranks);

        for (auto rank: coloring) { values_per_rank[rank]++; }

        return values_per_rank;
    }

    auto calc_displacements(const std::vector<int> &num_values_per_rank) {
        std::vector<int> displacements(num_values_per_rank.size(), 0);
        std::partial_sum(num_values_per_rank.begin(), num_values_per_rank.end() - 1,
                         displacements.begin() + 1);

        return displacements;
    }

    template<concepts::ContiguousContainer C>
    auto gather_in_root(const C &values, const MPI_Comm &comm, const MPI_Datatype &mpi_datatype) {
        using T = C::value_type;
        int num_ranks{};
        int rank{};

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int num_values{static_cast<int>(std::ranges::size(values))};
        std::vector<int> num_values_per_rank(num_ranks);
        MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
        int total_num_values =
                std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

        auto all_values = is_root(comm) ? std::vector<T>(total_num_values) : std::vector<T>{};
        auto displacements = calc_displacements(num_values_per_rank);

        MPI_Gatherv(std::ranges::data(values), num_values, mpi_datatype, all_values.data(),
                    num_values_per_rank.data(), displacements.data(), mpi_datatype, 0, comm);

        return all_values;
    }

    template<concepts::ContiguousContainer C>
    auto order_by_color(const C &values, const std::vector<rank_id> &coloring,
                        const std::vector<int> &displacements) {
        using T = C::value_type;
        const int num_ranks = static_cast<int>(displacements.size());

        if (std::ranges::size(values) != coloring.size()) {
            throw std::invalid_argument("Lenght of coloring and values do not match");
        }

        auto ordered_values = std::vector<T>(std::ranges::size(values));
        auto num_sorted_per_rank = std::vector(num_ranks, 0);
        for (int i = 0; i < coloring.size(); ++i) {
            const auto dest_rank = coloring[i];
            const auto dest_index = displacements[dest_rank] + num_sorted_per_rank[dest_rank];
            ordered_values[dest_index] = values[i];

            num_sorted_per_rank[dest_rank]++;
        }

        return ordered_values;
    }

    template<concepts::ContiguousContainer C>
    auto scatter_from_root(const C &values, const MPI_Comm &comm, const MPI_Datatype &mpi_datatype,
                           const std::vector<rank_id> &coloring) {
        using T = C::value_type;
        int num_ranks{};
        int rank{};
        const auto using_coloring = not coloring.empty();

        const int total_num_values = static_cast<int>(values.size());


        if (using_coloring and coloring.size() != total_num_values) {
            throw std::invalid_argument(
                    "Coloring being used, but size of coloring vector does not match size of data");
        }

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        auto new_num_values_per_rank =
                is_root(comm) ? internal::calc_num_values_per_rank(num_ranks, coloring)
                              : std::vector<int>(num_ranks);

        MPI_Bcast(new_num_values_per_rank.data(), static_cast<int>(new_num_values_per_rank.size()),
                  MPI_INT, 0, comm);

        const auto displacements = internal::calc_displacements(new_num_values_per_rank);
        const int new_num_values = new_num_values_per_rank[rank];

        auto values_to_scatter = using_coloring ? order_by_color(values, coloring, displacements)
                                                : std::vector<T>(std::ranges::begin(values),
                                                                 std::ranges::end(values));

        auto my_values = std::vector<T>(new_num_values);
        MPI_Scatterv(values_to_scatter.data(), new_num_values_per_rank.data(), displacements.data(),
                     mpi_datatype, my_values.data(), new_num_values, mpi_datatype, 0, comm);

        return my_values;
    }

    template<concepts::ContiguousContainer C>
        requires concepts::FundamentalType<typename C::value_type>
    auto gather_values_in_root(const C &values, const MPI_Comm &comm) {
        using T = C::value_type;
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        return gather_in_root(values, comm, mpi_datatype);
    }

    template<concepts::ContiguousContainer C>
        requires concepts::FundamentalType<typename C::value_type>
    auto scatter_values_from_root(const C &values, const MPI_Comm &comm,
                                  const std::vector<rank_id> &coloring) {
        using T = C::value_type;
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        return scatter_from_root(values, comm, mpi_datatype, coloring);
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_MPI_UTILS_HPP
