#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>
#include <numeric>
#include <algorithm>
#include <ranges>

#include "mpi_utils.hpp"
#include "concepts.hpp"

namespace reshuffle {
    template<ContiguousContainer C>
    requires Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &comm, const std::vector<int> &coloring = {}) {
        const auto using_coloring = not coloring.empty();
        if (using_coloring and coloring.size() != std::ranges::size(values)) {
            throw std::invalid_argument("Coloring being used, but size of coloring vector does not match size of data");
        }

        const auto all_coloring = internal::gather_values(coloring, comm);
        return internal::scatter_values(internal::gather_values(values, comm), comm, all_coloring);
    }

    template<ContiguousContainer C>
    requires Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::vector<int> &coloring = {}) {
        using T = C::value_type;
        if (not internal::mpi_comm_contains_root(origin_comm) or not internal::mpi_comm_contains_root(destiny_comm)) {
            throw std::invalid_argument("The root process must be included in both origin_comm and destiny_comm");
        }

        const auto using_coloring = not coloring.empty();
        if (using_coloring and coloring.size() != std::ranges::size(values)) {
            throw std::invalid_argument("Coloring being used, but size of coloring vector does not match size of data");
        }

        auto all_coloring = std::vector<int>{};
        auto all_values = std::vector<T>{};

        if (internal::in_mpi_comm(origin_comm)) {
            all_coloring = internal::gather_values(coloring, origin_comm);
            all_values = internal::gather_values(values, origin_comm);
        }

        const auto my_values = internal::in_mpi_comm(destiny_comm) ? internal::scatter_values(all_values,
                                                                                              destiny_comm,
                                                                                              all_coloring)
                                                                   : std::vector<T>{};

        return my_values;
    }

    template<Iterable I>
    requires Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &comm, const std::vector<int> &coloring = {}) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm, coloring);
    }

    template<Iterable I>
    requires Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::vector<int> &coloring = {}) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm, coloring);
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
