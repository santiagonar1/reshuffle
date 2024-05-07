#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <algorithm>
#include <mpi.h>
#include <numeric>
#include <ranges>
#include <vector>

#include "concepts.hpp"
#include "dimensions.hpp"
#include "mpi_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle {
    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &comm, const std::vector<rank_id> &coloring = {}) {
        const auto using_coloring = not coloring.empty();
        if (using_coloring and coloring.size() != std::ranges::size(values)) {
            throw std::invalid_argument(
                    "Coloring being used, but size of coloring vector does not match size of data");
        }

        const auto all_coloring = internal::gather_values_in_root(coloring, comm);
        return internal::scatter_values_from_root(internal::gather_values_in_root(values, comm),
                                                  comm, all_coloring);
    }

    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::vector<rank_id> &coloring = {}) {
        using T = C::value_type;
        if (not internal::mpi_comm_contains_root(origin_comm) or
            not internal::mpi_comm_contains_root(destiny_comm)) {
            throw std::invalid_argument(
                    "The root process must be included in both origin_comm and destiny_comm");
        }

        const auto using_coloring = not coloring.empty();
        if (using_coloring and coloring.size() != std::ranges::size(values)) {
            throw std::invalid_argument(
                    "Coloring being used, but size of coloring vector does not match size of data");
        }

        auto all_coloring = std::vector<rank_id>{};
        auto all_values = std::vector<T>{};

        if (internal::in_mpi_comm(origin_comm)) {
            all_coloring = internal::gather_values_in_root(coloring, origin_comm);
            all_values = internal::gather_values_in_root(values, origin_comm);
        }

        const auto my_values =
                internal::in_mpi_comm(destiny_comm)
                        ? internal::scatter_values_from_root(all_values, destiny_comm, all_coloring)
                        : std::vector<T>{};

        return my_values;
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &comm, const std::vector<rank_id> &coloring = {}) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm, coloring);
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::vector<rank_id> &coloring = {}) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm, coloring);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &comm, const std::vector<rank_id> &coloring,
                 const Dimensions2D &subdomain_dimension) {
        using T = M::value_type::value_type;

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = shuffle(buffer, comm, coloring);

        return internal::to_matrix(buffer, subdomain_dimension);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::vector<rank_id> &coloring, const Dimensions2D &subdomain_dimension) {
        using T = M::value_type::value_type;

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = shuffle(buffer, origin_comm, destiny_comm, coloring);

        return internal::to_matrix(buffer, subdomain_dimension);
    }
}// namespace reshuffle

#endif//RESHUFFLE_SHUFFLE_HPP
