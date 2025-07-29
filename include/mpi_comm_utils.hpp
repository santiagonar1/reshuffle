#ifndef MPI_COMM_UTILS_HPP
#define MPI_COMM_UTILS_HPP

#include <mpi.h>
#include <ranges>
#include <set>

#include "cartesian_product.hpp"
#include "communication_package.hpp"
#include "concepts.hpp"
#include "intercommunicator.hpp"
#include "mpi_utils.hpp"
#include "profiler.hpp"
#include "serialize.hpp"

namespace reshuffle::internal {

    template<std::size_t N>
    [[nodiscard]] auto
    get_number_of_receive_messages(const std::vector<MultidimensionalBlock<N>> &blocks_to_receive)
            -> int {
        PROFILE_SCOPE_NAMED("get_number_of_receive_messages");

        auto sources = std::set<Coordinates<N>>{};
        for (const auto &block: blocks_to_receive) { sources.insert(get_owner_coordinates(block)); }

        return sources.size();
    }

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
        auto serialized_values = serialize(values);
        MPI_Isend(serialized_values.data(), serialized_values.size(), MPI_BYTE, destiny, 0, comm,
                  &request);
        return request;
    }

    template<typename T>
        requires concepts::FundamentalType<T> || concepts::Serializable<T>
    auto async_send(std::span<T> values, const std::map<rank_id, LeftClosedRange> &data_mapping,
                    const Intercommunicator &intercomm) -> std::vector<MPI_Request> {
        auto send_requests = std::vector<MPI_Request>{};
        const auto comm = intercomm.get_intercommunicator();

        for (const auto &[rank, interval]: data_mapping) {
            const auto num_values = interval.get_length();
            // The rank here is relative to the final communicator
            const auto destiny = intercomm.get_intercomm_rank(
                    rank, Intercommunicator::SelectCommunicator::FINAL_COMM);
            auto values_to_send = std::span(values.data() + interval.get_left_bound(), num_values);
            send_requests.emplace_back(async_send(values_to_send, destiny, comm));
        }

        return send_requests;
    }

    template<concepts::FundamentalType T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<rank_id, std::vector<T>> {

        const auto [source, tag, count] =
                block_until_message_is_received(mpi::to_mpi_datatype<std::remove_cv_t<T>>(), comm);

        auto values = std::vector<T>(count);
        MPI_Recv(values.data(), count, mpi::to_mpi_datatype<std::remove_cv_t<T>>(), source, tag,
                 comm, MPI_STATUS_IGNORE);

        return {source, values};
    }

    template<concepts::NeedsSerialization T>
        requires concepts::Serializable<T>
    [[nodiscard]] auto block_receive(const MPI_Comm &comm) -> std::pair<rank_id, std::vector<T>> {
        const auto [source, tag, count] = block_until_message_is_received(MPI_BYTE, comm);

        auto buffer = std::vector<std::byte>(count);
        MPI_Recv(buffer.data(), count, MPI_BYTE, source, tag, comm, MPI_STATUS_IGNORE);

        auto values = deserialize<T>(buffer);
        return {source, values};
    }

    template<std::size_t N>
    auto get_dimensions(const std::array<std::vector<Block>, N> &blocks_to_receive)
            -> Dimensions<N> {

        if (blocks_to_receive[0].empty()) { return std::array<int, N>{}; }

        std::array<int, N> dimensions{};
        for (int i = 0; i < N; i++) {
            dimensions[i] = blocks_to_receive[i].back().get_interval().get_right_bound();
        }
        return dimensions;
    }

    template<typename T, typename Extents>
        requires concepts::FundamentalType<T> or concepts::Serializable<T>
    auto exchange_values(std::mdspan<const T, Extents> local_values,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_send,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_receive,
                         const ProcessorGrid<Extents::rank()> &initial_processor_grid,
                         const ProcessorGrid<Extents::rank()> &final_processor_grid,
                         const Intercommunicator &intercomm)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("exchange_values");

        constexpr auto N = Extents::rank();

        const auto dimensions = get_dimensions(blocks_to_receive);

        // TODO: See if I can omit this by making get_send_receive_package return MultidimensionalBlocks
        const auto multidimensional_blocks_to_send = get_cartesian_product(blocks_to_send);
        const auto multidimensional_blocks_to_receive = get_cartesian_product(blocks_to_receive);

        auto [send_buffer, intervals_to_send_per_rank] = get_send_package(
                local_values, multidimensional_blocks_to_send, final_processor_grid);

        auto num_messages_to_receive =
                get_number_of_receive_messages(multidimensional_blocks_to_receive);

        auto send_requests =
                async_send(std::span{send_buffer}, intervals_to_send_per_rank, intercomm);

        const auto num_new_local_values = get_num_elements(multidimensional_blocks_to_receive);
        auto new_local_values = std::vector<T>(num_new_local_values);
        for (int i = 0; i < num_messages_to_receive; i++) {
            const auto [source, data] = block_receive<T>(intercomm.get_intercommunicator());

            auto blocks_from_source =
                    multidimensional_blocks_to_receive |
                    std::views::filter([source, initial_processor_grid, intercomm](auto block) {
                        return initial_processor_grid.get_processor_id(get_owner_coordinates(
                                       block)) == intercomm.get_initial_comm_rank(source);
                    });

            auto received_values_view = std::span{data};
            for (const auto &block: blocks_from_source) {
                const auto num_values_in_block = get_num_elements(block);

                copy_data(received_values_view, std::mdspan(new_local_values.data(), dimensions),
                          block);

                received_values_view = received_values_view | std::views::drop(num_values_in_block);
            }
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);

        return {new_local_values, dimensions};
    }
}// namespace reshuffle::internal

#endif//MPI_COMM_UTILS_HPP
