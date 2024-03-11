#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>
#include <numeric>
#include <algorithm>
#include <ranges>
#include <zpp_bits.h>

#include "mpi_utils.hpp"
#include "concepts.hpp"

namespace reshuffle {
    template<ContiguousContainer C>
    requires FundamentalType<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &comm) {
        return internal::scatter_values(internal::gather_values(values, comm), comm);
    }

    template<ContiguousContainer C>
    requires FundamentalType<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        using T = C::value_type;
        if (not internal::mpi_comm_contains_root(origin_comm) or not internal::mpi_comm_contains_root(destiny_comm)) {
            throw std::invalid_argument("The root process must be included in both origin_comm and destiny_comm");
        }
        const auto all_values = internal::in_mpi_comm(origin_comm) ? internal::gather_values(values, origin_comm)
                                                                   : std::vector<T>{};
        const auto my_values = internal::in_mpi_comm(destiny_comm) ? internal::scatter_values(all_values,
                                                                                              destiny_comm)
                                                                   : std::vector<T>{};

        return my_values;
    }

    template<Iterable I>
    requires FundamentalType<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &comm) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm);
    }

    template<Iterable I>
    requires FundamentalType<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm);
    }

    template<Iterable I>
    requires Serializable<typename I::value_type> && (!FundamentalType<typename I::value_type>)
    auto shuffle(const I &values, const MPI_Comm &comm) {
        using T = I::value_type;

        T dummy{};
        auto [dummy_data, dummy_out] = zpp::bits::data_out();
        dummy_out(dummy).or_throw();
        const auto num_bytes_type = dummy_data.size();

        auto [data, out] = zpp::bits::data_out();
        for (auto &v: values) {
            out(v).or_throw();
        }

        const auto new_values = internal::scatter_values(internal::gather_values(data, comm), num_bytes_type, comm);

        zpp::bits::in in(new_values);
        std::vector<T> results(new_values.size() / num_bytes_type);
        for (auto &v: results) {
            in(v).or_throw();
        }

        return results;
    }

}

#endif //RESHUFFLE_SHUFFLE_HPP
