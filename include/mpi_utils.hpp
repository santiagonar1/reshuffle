#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <algorithm>
#include <mpi.h>
#include <stdexcept>
#include <vector>

#include "concepts.hpp"
#include "rank_id.hpp"
#include "serialize.hpp"

namespace reshuffle::mpi {

    namespace internal {
        struct ReceivedMessageInformation {
            int source;
            int tag;
            int num_elements;
        };

        [[nodiscard]] auto get_num_elements(const MPI_Status &status, const MPI_Datatype &datatype)
                -> int;

        [[nodiscard]] auto block_until_message_is_received(const MPI_Datatype &datatype,
                                                           const MPI_Comm &comm)
                -> ReceivedMessageInformation;
    }// namespace internal

    template<typename DATATYPE>
    [[nodiscard]] MPI_Datatype to_mpi_datatype() {
        if (std::is_same_v<DATATYPE, int>) { return MPI_INT; }
        if (std::is_same_v<DATATYPE, float>) { return MPI_FLOAT; }
        if (std::is_same_v<DATATYPE, double>) { return MPI_DOUBLE; }
        if (std::is_same_v<DATATYPE, std::byte>) { return MPI_BYTE; }
        if (std::is_same_v<DATATYPE, char>) { return MPI_CHAR; }
        if (std::is_same_v<DATATYPE, unsigned char>) { return MPI_UNSIGNED_CHAR; }
        if (std::is_same_v<DATATYPE, short>) { return MPI_SHORT; }
        if (std::is_same_v<DATATYPE, unsigned short>) { return MPI_UNSIGNED_SHORT; }
        if (std::is_same_v<DATATYPE, unsigned int>) { return MPI_UNSIGNED; }
        if (std::is_same_v<DATATYPE, long>) { return MPI_LONG; }
        if (std::is_same_v<DATATYPE, unsigned long>) { return MPI_UNSIGNED_LONG; }
        if (std::is_same_v<DATATYPE, long long>) { return MPI_LONG_LONG; }
        if (std::is_same_v<DATATYPE, unsigned long long>) { return MPI_UNSIGNED_LONG_LONG; }
        if (std::is_same_v<DATATYPE, bool>) { return MPI_C_BOOL; }
        if (std::is_same_v<DATATYPE, long double>) { return MPI_LONG_DOUBLE; }


        throw std::invalid_argument("No MPI Datatype");
    }

    [[nodiscard]] auto get_rank_id(const MPI_Comm &comm) -> rank_id;

    [[nodiscard]] auto is_root(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_num_ranks(const MPI_Comm &comm) -> int;

    [[nodiscard]] auto is_comm_null(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_sub_comm(const MPI_Comm &base_comm, const std::vector<rank_id> &ranks)
            -> MPI_Comm;

    [[nodiscard]] auto get_group(const MPI_Comm &comm) -> std::optional<MPI_Group>;

    [[nodiscard]] auto belongs_to_comm(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_sub_group(const MPI_Group &group, const std::vector<rank_id> &ranks)
            -> MPI_Group;

    [[nodiscard]] auto is_sub_comm(const MPI_Comm &comm, const MPI_Comm &possible_sub_comm) -> bool;

    template<concepts::FundamentalType T>
    [[nodiscard]] auto async_send(std::span<T> values, const rank_id destiny, const MPI_Comm &comm)
            -> MPI_Request {
        auto request = MPI_Request{};
        MPI_Isend(values.data(), values.size(), mpi::to_mpi_datatype<std::remove_cv_t<T>>(),
                  destiny, 0, comm, &request);
        return request;
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto async_send(std::span<T> values, const rank_id destiny, const MPI_Comm &comm)
            -> MPI_Request {
        auto request = MPI_Request{};
        auto serialized_values = reshuffle::internal::serialize(values);
        MPI_Isend(serialized_values.data(), serialized_values.size(), MPI_BYTE, destiny, 0, comm,
                  &request);
        return request;
    }

    template<concepts::FundamentalType T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<rank_id, std::vector<T>> {

        const auto [source, tag, count] = internal::block_until_message_is_received(
                mpi::to_mpi_datatype<std::remove_cv_t<T>>(), comm);

        auto values = std::vector<T>(count);
        MPI_Recv(values.data(), count, mpi::to_mpi_datatype<std::remove_cv_t<T>>(), source, tag,
                 comm, MPI_STATUS_IGNORE);

        return {source, values};
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<rank_id, std::vector<T>> {
        const auto [source, tag, count] = internal::block_until_message_is_received(MPI_BYTE, comm);

        auto buffer = std::vector<std::byte>(count);
        MPI_Recv(buffer.data(), count, MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);

        auto values = reshuffle::internal::deserialize<T>(buffer);
        return {source, values};
    }

}// namespace reshuffle::mpi

#endif//RESHUFFLE_MPI_UTILS_HPP
