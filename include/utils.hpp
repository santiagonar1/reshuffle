#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include <mpi.h>
#include <zpp_bits.h>
#include <ranges>

#include "concepts.hpp"

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

    template<Iterable I>
    requires Serializable<typename I::value_type>
    auto serialize(const I &values) {
        auto [data, out] = zpp::bits::data_out();
        std::ranges::for_each(values, [&out](auto &v) { out(v).or_throw(); });

        return data;
    }

    template<NonTriviallySerializable T>
    auto make_object() {
        return T::create();
    }

    template<DefaultConstructible T>
    auto make_object() {
        return T{};
    }


    template<Serializable S>
    auto deserialize(const std::vector<std::byte> &bytes) {
        const auto num_bytes_type = sizeof(S);
        std::vector<S> values(bytes.size() / num_bytes_type, make_object<S>());
        auto in = zpp::bits::in(bytes);
        std::ranges::for_each(values, [&in](auto &v) { in(v).or_throw(); });

        return values;
    }
}

#endif //RESHUFFLE_UTILS_HPP
