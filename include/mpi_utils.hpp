#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <algorithm>
#include <mpi.h>
#include <optional>
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

    template<concepts::MPIType DATATYPE>
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

    [[nodiscard]] auto get_rank_id(const MPI_Comm &comm) -> std::optional<RankId>;

    [[nodiscard]] auto is_root(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_num_ranks(const MPI_Comm &comm) -> int;

    [[nodiscard]] auto is_comm_null(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_sub_comm(const MPI_Comm &base_comm, const std::vector<RankId> &ranks)
            -> MPI_Comm;

    [[nodiscard]] auto get_group(const MPI_Comm &comm) -> std::optional<MPI_Group>;

    [[nodiscard]] auto belongs_to_comm(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_sub_group(const MPI_Group &group, const std::vector<RankId> &ranks)
            -> MPI_Group;

    [[nodiscard]] auto is_sub_comm(const MPI_Comm &comm, const MPI_Comm &possible_sub_comm) -> bool;

    template<concepts::MPIType T>
    [[nodiscard]] auto async_send(std::span<T> values, const RankId destiny, const MPI_Comm &comm)
            -> MPI_Request {
        auto request = MPI_Request{};
        MPI_Isend(values.data(), values.size(), mpi::to_mpi_datatype<std::remove_cv_t<T>>(),
                  destiny, 0, comm, &request);
        return request;
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto async_send(std::span<T> values, const RankId destiny, const MPI_Comm &comm)
            -> MPI_Request {
        auto request = MPI_Request{};
        auto serialized_values = reshuffle::internal::serialize(values);
        MPI_Isend(serialized_values.data(), serialized_values.size(), MPI_BYTE, destiny, 0, comm,
                  &request);
        // TODO: Make this a true async operation
        // This is a hot fix. The issue here is that the serialized_values vector is local to this
        // function. This means that its value might be dropped before the MPI_Isend is executed.
        // Adding the MPI_Wait here forces the execution of the MPI_Isend, but makes the function
        // sync. A proper implementation should return a wrapper that owns both the serialized values
        // and the request, so that we guarantee this is not dropped.
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        return request;
    }

    template<concepts::MPIType T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<RankId, std::vector<T>> {

        const auto [source, tag, count] = internal::block_until_message_is_received(
                mpi::to_mpi_datatype<std::remove_cv_t<T>>(), comm);

        auto values = std::vector<T>(count);
        MPI_Recv(values.data(), count, mpi::to_mpi_datatype<std::remove_cv_t<T>>(), source, tag,
                 comm, MPI_STATUS_IGNORE);

        return {source, values};
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<RankId, std::vector<T>> {
        const auto [source, tag, count] = internal::block_until_message_is_received(MPI_BYTE, comm);

        auto buffer = std::vector<std::byte>(count);
        MPI_Recv(buffer.data(), count, MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);

        auto values = reshuffle::internal::deserialize<T>(buffer);
        return {source, values};
    }

    template<concepts::MPIType T>
    [[nodiscard]] auto block_scatter(std::span<T> values,
                                     const std::map<RankId, int> &values_per_rank,
                                     const RankId root, const MPI_Comm &comm) -> std::vector<T> {
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

    namespace internal {
        template<typename T>
        [[nodiscard]] auto get_send_buffer_and_mapping(std::span<T> values,
                                                       const std::map<RankId, int> &values_per_rank,
                                                       const int num_ranks, const RankId root)
                -> std::tuple<std::vector<T>, std::vector<std::byte>, std::map<RankId, int>> {
            auto bytes_per_rank = std::map<RankId, int>{};
            auto bytes_to_send = std::vector<std::byte>{};
            auto num_values_per_rank = std::vector<int>(num_ranks);

            for (int i = 0; i < num_ranks; ++i) {
                num_values_per_rank[i] = reshuffle::internal::find(values_per_rank, i).value_or(0);
            }

            // TODO: This can be simplified if we have a T with fixed size
            auto num_values_serialized{0};
            for (int i = 0; i < root; ++i) {
                const auto bytes = reshuffle::internal::serialize(
                        values.subspan(num_values_serialized, num_values_per_rank[i]));
                bytes_per_rank[i] = static_cast<int>(bytes.size());
                bytes_to_send.append_range(bytes);
                num_values_serialized += num_values_per_rank[i];
            }

            bytes_per_rank[root] = 0;
            const auto num_values_to_send_myself = num_values_per_rank[root];
            const auto my_values = std::vector<T>{values.begin() + num_values_serialized,
                                                  values.begin() + num_values_serialized +
                                                          num_values_to_send_myself};
            num_values_serialized += num_values_per_rank[root];

            for (int i = root + 1; i < num_values_per_rank.size(); ++i) {
                const auto bytes = reshuffle::internal::serialize(
                        values.subspan(num_values_serialized, num_values_per_rank[i]));
                bytes_per_rank[i] = static_cast<int>(bytes.size());
                bytes_to_send.append_range(bytes);
                num_values_serialized += num_values_per_rank[i];
            }

            return {my_values, bytes_to_send, bytes_per_rank};
        }

        template<concepts::FixedSizeSerializable T>
        [[nodiscard]] auto get_num_bytes() -> int {
            const auto dummy_vector = std::vector<T>(1);
            return static_cast<int>(reshuffle::internal::serialize(dummy_vector).size());
        }

        template<concepts::FixedSizeSerializable T>
        [[nodiscard]] auto get_send_buffer_and_mapping(std::span<T> values,
                                                       const std::map<RankId, int> &values_per_rank,
                                                       const int num_ranks, const RankId root)
                -> std::tuple<std::vector<T>, std::vector<std::byte>, std::map<RankId, int>> {
            auto bytes_per_rank = std::map<RankId, int>{};
            const auto dummy_vector = std::vector<T>(1);
            const auto num_bytes_per_value = get_num_bytes<T>();
            auto num_values_per_rank = std::vector<int>(num_ranks);

            for (int i = 0; i < num_ranks; ++i) {
                num_values_per_rank[i] = reshuffle::internal::find(values_per_rank, i).value_or(0);
                bytes_per_rank[i] = num_values_per_rank[i] * num_bytes_per_value;
            }

            const auto num_bytes_to_send_to_myself = bytes_per_rank[root];
            const auto buffer_size =
                    values.size() * num_bytes_per_value - num_bytes_to_send_to_myself;
            auto bytes_to_send = std::vector<std::byte>{buffer_size};

            auto accumulated_values = std::vector<int>(num_ranks + 1);
            std::partial_sum(num_values_per_rank.begin(), num_values_per_rank.end(),
                             accumulated_values.begin() + 1);

            const auto range_values_in_root =
                    std::pair{accumulated_values[root], accumulated_values[root + 1]};
            const auto num_serialized_values_before_root =
                    range_values_in_root.first * num_bytes_per_value;

            reshuffle::internal::serialize(values.subspan(0, range_values_in_root.first),
                                           std::span{bytes_to_send});
            reshuffle::internal::serialize(
                    values.subspan(range_values_in_root.second),
                    std::span{bytes_to_send.begin() + num_serialized_values_before_root,
                              bytes_to_send.end()});

            bytes_per_rank[root] = 0;
            const auto my_values = std::vector<T>{values.begin() + range_values_in_root.first,
                                                  values.begin() + range_values_in_root.second};

            return {my_values, bytes_to_send, bytes_per_rank};
        }
    }// namespace internal

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto block_scatter(std::span<T> values,
                                     const std::map<RankId, int> &values_per_rank,
                                     const RankId root, const MPI_Comm &comm) -> std::vector<T> {
        const auto rank = get_rank_id(comm);


        if (rank == root) {
            const auto num_ranks = get_num_ranks(comm);
            auto [my_values, bytes_to_send, bytes_per_rank] =
                    internal::get_send_buffer_and_mapping(values, values_per_rank, num_ranks, root);

            const auto _ = block_scatter(std::span{bytes_to_send}, bytes_per_rank, root, comm);

            return my_values;
        }

        auto dummy_bytes = std::vector<std::byte>{};
        const auto dummy_values_per_rank = std::map<RankId, int>{};
        const auto received_bytes =
                block_scatter(std::span{dummy_bytes}, dummy_values_per_rank, root, comm);

        return reshuffle::internal::deserialize<T>(received_bytes);
    }

}// namespace reshuffle::mpi

#endif//RESHUFFLE_MPI_UTILS_HPP
