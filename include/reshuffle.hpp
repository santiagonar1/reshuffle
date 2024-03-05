#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>
#include <numeric>

#include "mpi_utils.hpp"
#include "concepts.hpp"

namespace reshuffle {
    template<Container C>
    auto shuffle(const C &values, const MPI_Comm &comm) {
        return internal::scatter_values(internal::gather_values(values, comm), comm);
    }

    template<Container C>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        using T = C::value_type;
        if (not internal::is_root_in_mpi_comm(origin_comm) or not internal::is_root_in_mpi_comm(destiny_comm)) {
            throw std::invalid_argument("The root process must be included in both origin_comm and destiny_comm");
        }
        const auto all_values = internal::in_mpi_comm(origin_comm) ? internal::gather_values(values, origin_comm)
                                                                   : std::vector<T>{};
        const auto my_values = internal::in_mpi_comm(destiny_comm) ? internal::scatter_values(all_values,
                                                                                              destiny_comm)
                                                                   : std::vector<T>{};

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
