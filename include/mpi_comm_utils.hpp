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

    template<typename T>
        requires concepts::FundamentalType<T> || concepts::Serializable<T>
    auto async_send(std::span<T> values, const std::map<RankId, LeftClosedRange> &data_mapping,
                    const Intercommunicator &intercomm) -> std::vector<MPI_Request> {
        auto send_requests = std::vector<MPI_Request>{};
        const auto comm = intercomm.get_intercommunicator();

        for (const auto &[rank, interval]: data_mapping) {
            const auto num_values = interval.get_length();
            // The rank here is relative to the final communicator
            const auto destiny = intercomm.get_intercomm_rank(
                    rank, Intercommunicator::SelectCommunicator::FINAL_COMM);
            auto values_to_send = std::span(values.data() + interval.get_left_bound(), num_values);
            send_requests.emplace_back(mpi::async_send(values_to_send, destiny, comm));
        }

        return send_requests;
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
    auto process_received_blocks(std::span<const T> received_values,
                                 std::mdspan<T, Extents> destination, auto &blocks_from_source)
            -> void {
        auto received_values_view = received_values;

        for (const auto &block: blocks_from_source) {
            const auto num_values_in_block = get_num_elements(block);

            copy_data(received_values_view, destination, block);

            received_values_view = received_values_view | std::views::drop(num_values_in_block);
        }
    }

    template<typename T, typename Extents>
        requires concepts::FundamentalType<T> or concepts::Serializable<T>
    auto scatter_values(std::mdspan<const T, Extents> local_values,
                        const std::array<std::vector<Block>, Extents::rank()> &blocks_to_send,
                        const std::array<std::vector<Block>, Extents::rank()> &blocks_to_receive,
                        const ProcessorGrid<Extents::rank()> &final_processor_grid,
                        const RankId root, const Intercommunicator &intercomm)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("scatter_values");

        const auto multidimensional_blocks_to_send = get_cartesian_product(blocks_to_send);
        auto [send_buffer, intervals_to_send_per_rank] = get_send_package(
                local_values, multidimensional_blocks_to_send, final_processor_grid);

        auto values_per_rank = std::map<RankId, int>{};
        for (const auto &[rank, interval]: intervals_to_send_per_rank) {
            values_per_rank.emplace(rank, interval.get_length());
        }

        const auto values = mpi::block_scatter(std::span{send_buffer}, values_per_rank, root,
                                               intercomm.get_intercommunicator());
        const auto dimensions = get_dimensions(blocks_to_receive);

        return {values, dimensions};
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

        const auto intercomm_rank = mpi::get_rank_id(intercomm.get_intercommunicator()).value();
        const auto rank_final_comm =
                intercomm.get_final_comm_rank(intercomm_rank).value_or(INVALID_RANK_ID);
        const auto send_to_myself_optional = find(intervals_to_send_per_rank, rank_final_comm);
        intervals_to_send_per_rank.erase(rank_final_comm);

        auto num_messages_to_receive =
                get_number_of_receive_messages(multidimensional_blocks_to_receive);

        if (send_to_myself_optional.has_value()) { num_messages_to_receive -= 1; }

        auto send_requests =
                async_send(std::span{send_buffer}, intervals_to_send_per_rank, intercomm);

        const auto num_new_local_values = get_num_elements(multidimensional_blocks_to_receive);
        auto new_local_values = std::vector<T>(num_new_local_values);

        if (send_to_myself_optional.has_value()) {
            const auto send_to_myself = send_to_myself_optional.value();
            auto blocks_from_source = multidimensional_blocks_to_receive |
                                      std::views::filter([intercomm_rank, initial_processor_grid,
                                                          intercomm](auto block) {
                                          return initial_processor_grid.get_processor_id(
                                                         get_owner_coordinates(block)) ==
                                                 intercomm.get_initial_comm_rank(intercomm_rank);
                                      });

            auto received_values_view =
                    std::span{std::as_const(send_buffer).data() + send_to_myself.get_left_bound(),
                              static_cast<size_t>(send_to_myself.get_length())};
            process_received_blocks(received_values_view,
                                    std::mdspan(new_local_values.data(), dimensions),
                                    blocks_from_source);
        }


        for (int i = 0; i < num_messages_to_receive; i++) {
            const auto [source, data] = mpi::block_receive<T>(intercomm.get_intercommunicator());

            auto blocks_from_source =
                    multidimensional_blocks_to_receive |
                    std::views::filter([source, initial_processor_grid, intercomm](auto block) {
                        return initial_processor_grid.get_processor_id(get_owner_coordinates(
                                       block)) == intercomm.get_initial_comm_rank(source);
                    });

            auto received_values_view = std::span{data};
            process_received_blocks(received_values_view,
                                    std::mdspan(new_local_values.data(), dimensions),
                                    blocks_from_source);
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);

        return {new_local_values, dimensions};
    }
}// namespace reshuffle::internal

#endif//MPI_COMM_UTILS_HPP
