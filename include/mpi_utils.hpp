#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <algorithm>
#include <mpi.h>
#include <numeric>
#include <ranges>
#include <span>
#include <vector>

#include "coloring_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle::internal {
    template<typename DATATYPE>
    [[nodiscard]] MPI_Datatype to_mpi_datatype() {
        if (std::is_same_v<DATATYPE, int>) { return MPI_INT; }

        if (std::is_same_v<DATATYPE, float>) { return MPI_FLOAT; }

        if (std::is_same_v<DATATYPE, double>) { return MPI_DOUBLE; }

        if (std::is_same_v<DATATYPE, std::byte>) { return MPI_BYTE; }

        throw std::invalid_argument("No MPI Datatype");
    }

    [[nodiscard]] auto get_rank_id(const MPI_Comm &comm) -> rank_id;

    [[nodiscard]] auto in_mpi_comm(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto is_root(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_num_ranks(const MPI_Comm &comm) -> int;

    [[nodiscard]] auto is_comm_null(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_displacements(const std::vector<int> &num_values_per_rank)
            -> std::vector<int>;

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto gather_in_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                      const MPI_Datatype &mpi_datatype,
                                      const std::vector<rank_id> &global_coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;

        const auto num_ranks = get_num_ranks(comm);
        const auto using_coloring = not global_coloring.empty();

        const int num_values{static_cast<int>(std::ranges::size(values))};
        std::vector<int> num_values_per_rank(num_ranks);
        MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
        int total_num_values =
                std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

        auto all_values = is_root(comm) ? std::vector<T>(total_num_values) : std::vector<T>{};
        const auto displacements = get_displacements(num_values_per_rank);

        MPI_Gatherv(std::ranges::data(values), num_values, mpi_datatype, all_values.data(),
                    num_values_per_rank.data(), displacements.data(), mpi_datatype, 0, comm);

        if (using_coloring and is_root(comm)) {
            const auto indices = get_global_index_by_rank(global_coloring, num_ranks);
            all_values = reorder_values(all_values, indices);
        }

        return all_values;
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto scatter_from_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                         const MPI_Datatype &mpi_datatype,
                                         const std::vector<rank_id> &coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
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

        const auto displacements = internal::get_displacements(new_num_values_per_rank);
        const int new_num_values = new_num_values_per_rank[rank];

        auto values_to_scatter = using_coloring ? order_by_color(values, coloring, displacements)
                                                : std::vector<T>(std::ranges::begin(values),
                                                                 std::ranges::end(values));

        auto my_values = std::vector<T>(new_num_values);
        MPI_Scatterv(values_to_scatter.data(), new_num_values_per_rank.data(), displacements.data(),
                     mpi_datatype, my_values.data(), new_num_values, mpi_datatype, 0, comm);

        return my_values;
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto gather_in_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                             const std::vector<rank_id> &global_coloring = {})
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        return gather_in_root(values, comm, mpi_datatype, global_coloring);
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto scatter_values_from_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                                const std::vector<rank_id> &coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        return scatter_from_root(values, comm, mpi_datatype, coloring);
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_MPI_UTILS_HPP
