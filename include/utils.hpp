#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include <mpi.h>
#include <ranges>
#include <zpp_bits.h>

#include "concepts.hpp"
#include "dimensions.hpp"

namespace reshuffle::internal {
    template<typename DATATYPE>
    MPI_Datatype to_mpi_datatype() {
        if (std::is_same_v<DATATYPE, int>) {
            return MPI_INT;
        } else if (std::is_same_v<DATATYPE, float>) {
            return MPI_FLOAT;
        } else if (std::is_same_v<DATATYPE, double>) {
            return MPI_DOUBLE;
        } else if (std::is_same_v<DATATYPE, std::byte>) {
            return MPI_BYTE;
        }

        throw std::invalid_argument("No MPI Datatype");
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type>
    auto serialize(const I &values) {
        auto [data, out] = zpp::bits::data_out();
        std::ranges::for_each(values, [&out](auto &v) { out(v).or_throw(); });

        return data;
    }

    template<concepts::NonTriviallySerializable T>
    auto make_object() {
        return T::create();
    }

    template<concepts::DefaultConstructible T>
    auto make_object() {
        return T{};
    }

    template<concepts::Serializable S>
    auto deserialize(const std::vector<std::byte> &bytes) {
        const auto num_bytes_type = sizeof(S);
        std::vector<S> values(bytes.size() / num_bytes_type, make_object<S>());
        auto in = zpp::bits::in(bytes);
        std::ranges::for_each(values, [&in](auto &v) { in(v).or_throw(); });

        return values;
    }

    template<typename T, typename U>
    auto combine(const std::vector<T> &first, const std::vector<U> &second) {
        std::vector<std::pair<T, U>> combination{};
        for (const auto &v2: second) {
            for (const auto &v1: first) { combination.emplace_back(v1, v2); }
        }

        return combination;
    }

    template<typename T>
    auto to_matrix(const std::vector<T> &values, const Dimension<2> &dimension) {
        using Matrix = std::vector<std::vector<T>>;

        auto matrix = Matrix(dimension.get_num_values_dim(1),
                             std::vector<T>(dimension.get_num_values_dim(0)));
        int i = 0;
        for (auto &row: matrix) {
            for (auto &value: row) {
                value = values[i];
                ++i;
            }
        }

        return matrix;
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_UTILS_HPP
