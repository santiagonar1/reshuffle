#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <algorithm>
#include <mpi.h>
#include <stdexcept>
#include <vector>

#include "concepts.hpp"
#include "rank_id.hpp"
#include "serialize.hpp"
#include "utils.hpp"

#include <map>

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

    template<concepts::FundamentalType T>
    [[nodiscard]] auto block_scatter(std::span<T> values,
                                     const std::map<rank_id, int> &values_per_rank,
                                     const rank_id root, const MPI_Comm &comm) -> std::vector<T> {
        const auto num_ranks = get_num_ranks(comm);

        auto num_values_per_rank = std::vector<int>(num_ranks);
        for (int i = 0; i < num_ranks; ++i) {
            num_values_per_rank[i] = reshuffle::internal::find(values_per_rank, i).value_or(0);
        }

        auto num_values_to_receive{0};
        MPI_Scatter(num_values_per_rank.data(), 1, MPI_INT, &num_values_to_receive, 1, MPI_INT,
                    root, comm);

        auto displacements = std::vector<int>(num_ranks + 1);
        std::partial_sum(num_values_per_rank.begin(), num_values_per_rank.end(),
                         displacements.begin() + 1);

        auto received_values = std::vector<T>(num_values_to_receive);
        MPI_Scatterv(values.data(), num_values_per_rank.data(), displacements.data(),
                     to_mpi_datatype<T>(), received_values.data(), num_values_to_receive,
                     to_mpi_datatype<T>(), root, comm);

        return received_values;
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto block_scatter(std::span<T> values,
                                     const std::map<rank_id, int> &values_per_rank,
                                     const rank_id root, const MPI_Comm &comm) -> std::vector<T> {
        const auto num_ranks = get_num_ranks(comm);
        const auto rank = get_rank_id(comm);

        auto num_values_per_rank = std::vector<int>(num_ranks);
        auto num_bytes_per_rank = std::vector<int>(num_ranks);
        auto send_buffer = std::vector<std::byte>{};


        if (rank == root) {
            for (int i = 0; i < num_ranks; ++i) {
                num_values_per_rank[i] = reshuffle::internal::find(values_per_rank, i).value_or(0);
            }

            // TODO: This can be simplified if we have a T with fixed size
            auto num_values_serialized{0};
            for (int i = 0; i < num_values_per_rank.size(); ++i) {
                const auto bytes = reshuffle::internal::serialize(
                        values.subspan(num_values_serialized, num_values_per_rank[i]));
                num_bytes_per_rank[i] = static_cast<int>(bytes.size());
                send_buffer.append_range(bytes);
                num_values_serialized += num_values_per_rank[i];
            }
        }

        auto num_bytes_to_receive{0};
        MPI_Scatter(num_bytes_per_rank.data(), 1, MPI_INT, &num_bytes_to_receive, 1, MPI_INT, root,
                    comm);

        auto displacements = std::vector<int>(num_ranks + 1);
        std::partial_sum(num_bytes_per_rank.begin(), num_bytes_per_rank.end(),
                         displacements.begin() + 1);

        auto received_bytes = std::vector<std::byte>(num_bytes_to_receive);
        MPI_Scatterv(send_buffer.data(), num_bytes_per_rank.data(), displacements.data(), MPI_BYTE,
                     received_bytes.data(), num_bytes_to_receive, MPI_BYTE, root, comm);

        return reshuffle::internal::deserialize<T>(received_bytes);
    }

}// namespace reshuffle::mpi

#endif//RESHUFFLE_MPI_UTILS_HPP
