#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <mpi.h>
#include <vector>
#include <numeric>
#include <ranges>

#include "utils.hpp"
#include "concepts.hpp"

namespace reshuffle::internal {
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

    template<ContiguousContainer C>
    requires FundamentalType<typename C::value_type>
    auto gather_values(const C &values, const MPI_Comm &comm) {
        using T = C::value_type;
        int num_ranks{};
        int rank{};
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int num_values{static_cast<int>(std::ranges::size(values))};
        std::vector<int> num_values_per_rank(num_ranks);
        MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
        int total_num_values = std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

        auto all_values = rank == 0 ? std::vector<T>(total_num_values) : std::vector<T>{};
        auto displacements = calc_displacements(num_values_per_rank);
        MPI_Gatherv(std::ranges::data(values), num_values, mpi_datatype, all_values.data(), num_values_per_rank.data(),
                    displacements.data(), mpi_datatype, 0, comm);

        return all_values;
    }

    template<ContiguousContainer C>
    requires Serializable<typename C::value_type> && (not FundamentalType<typename C::value_type>)
    auto gather_values(const C &values, const MPI_Comm &comm) {
        using T = C::value_type;
        const auto data = serialize(values);

        int num_ranks{};
        int rank{};
        MPI_Datatype mpi_datatype = MPI_BYTE;

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int num_values{static_cast<int>(std::ranges::size(data))};
        std::vector<int> num_values_per_rank(num_ranks);
        MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
        int total_num_values = std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

        auto all_data = rank == 0 ? std::vector<std::byte>(total_num_values) : std::vector<std::byte>{};
        auto displacements = calc_displacements(num_values_per_rank);
        MPI_Gatherv(std::ranges::data(data), num_values, mpi_datatype, all_data.data(), num_values_per_rank.data(),
                    displacements.data(), mpi_datatype, 0, comm);

        return deserialize<T>(all_data);
    }

    template<ContiguousContainer C>
    requires FundamentalType<typename C::value_type>
    auto scatter_values(const C &values, const MPI_Comm &comm) {
        using T = C::value_type;
        int num_ranks{};
        int rank{};
        MPI_Datatype mpi_datatype = internal::to_mpi_datatype<T>();

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        int total_num_values = static_cast<int>(std::ranges::size(values));
        MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);

        auto new_num_values_per_rank = internal::calc_num_values_per_rank(total_num_values, num_ranks);
        const auto displacements = internal::calc_displacements(new_num_values_per_rank);
        const int new_num_values = new_num_values_per_rank[rank];

        std::vector<T> my_values(new_num_values);
        MPI_Scatterv(std::ranges::data(values), new_num_values_per_rank.data(), displacements.data(), mpi_datatype,
                     my_values.data(),
                     new_num_values, mpi_datatype, 0, comm);

        return my_values;
    }

    template<ContiguousContainer C>
    requires Serializable<typename C::value_type> && (not FundamentalType<typename C::value_type>)
    auto scatter_values(const C &values, const MPI_Comm &comm) {
        using T = C::value_type;
        int num_ranks{};
        int rank{};

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        const auto num_bytes_type = sizeof(T);
        const auto data = serialize(values);

        MPI_Datatype mpi_datatype;
        MPI_Type_contiguous(num_bytes_type, MPI_BYTE, &mpi_datatype);
        MPI_Type_commit(&mpi_datatype);

        int total_num_values = static_cast<int>(std::ranges::size(values));
        MPI_Bcast(&total_num_values, 1, MPI_INT, 0, comm);

        auto new_num_values_per_rank = internal::calc_num_values_per_rank(total_num_values, num_ranks);
        const auto displacements = internal::calc_displacements(new_num_values_per_rank);
        const int new_num_values = new_num_values_per_rank[rank];

        auto my_data = std::vector<std::byte>(new_num_values * num_bytes_type);
        MPI_Scatterv(std::ranges::data(data), new_num_values_per_rank.data(), displacements.data(), mpi_datatype,
                     my_data.data(),
                     new_num_values, mpi_datatype, 0, comm);

        return deserialize<T>(my_data);
    }

    auto in_mpi_comm(const MPI_Comm &comm) {
        int rank{};
        MPI_Errhandler err_handler{};
        MPI_Comm_get_errhandler(MPI_COMM_WORLD, &err_handler);
        MPI_Comm_set_errhandler(MPI_COMM_WORLD, MPI_ERRORS_RETURN);
        const auto err = MPI_Comm_rank(comm, &rank);
        MPI_Comm_set_errhandler(MPI_COMM_WORLD, err_handler);
        MPI_Errhandler_free(&err_handler);

        return err != MPI_ERR_COMM;
    }

    auto is_root() {
        int rank{};
        MPI_Comm_rank(MPI_COMM_WORLD, &rank);
        return rank == 0;
    }

    auto mpi_comm_contains_root(const MPI_Comm &comm) {
        auto root_in_comm = is_root() && in_mpi_comm(comm);
        // TODO: Check if we should use something else besides MPI_COMM_WORLD here (maybe simply comm?).
        MPI_Bcast(&root_in_comm, 1, MPI_CXX_BOOL, 0, MPI_COMM_WORLD);
        return root_in_comm;
    }
}

#endif //RESHUFFLE_MPI_UTILS_HPP
