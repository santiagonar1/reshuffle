#ifndef MPI_COMM_UTILS_HPP
#define MPI_COMM_UTILS_HPP

#include <mpi.h>
#include <ranges>
#include <set>

#include "communication_package.hpp"
#include "concepts.hpp"
#include "inter_communicator.hpp"
#include "interval.hpp"
#include "mpi_utils.hpp"
#include "profiler.hpp"

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
    auto async_send(std::span<T> values, const std::map<RankId, Interval> &data_mapping,
                    const InterCommunicator &inter_communicator) -> std::vector<MPI_Request> {
        auto send_requests = std::vector<MPI_Request>{};
        const auto comm = inter_communicator.get_inter_communicator();

        for (const auto &[rank, interval]: data_mapping) {
            const auto num_values = interval.get_length();
            // The rank here is relative to the final communicator
            const auto destiny = inter_communicator.get_inter_comm_rank(
                    rank, InterCommunicator::SelectCommunicator::FINAL_COMM);
            auto values_to_send = std::span(values.data() + interval.get_left_bound(), num_values);
            send_requests.emplace_back(mpi::async_send(values_to_send, destiny, comm));
        }

        return send_requests;
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

    template<concepts::Exchangeable T, typename Extents>
    [[nodiscard]] auto
    scatter_values(std::mdspan<const T, Extents> local_values,
                   const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_send,
                   const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_receive,
                   const ProcessorGrid<Extents::rank()> &final_processor_grid, const RankId root,
                   const InterCommunicator &inter_communicator)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("scatter_values");

        auto [send_buffer, intervals_to_send_per_rank] =
                get_send_package(local_values, blocks_to_send, final_processor_grid);

        auto values_per_rank = std::map<RankId, int>{};
        for (const auto &[rank, interval]: intervals_to_send_per_rank) {
            values_per_rank.emplace(rank, interval.get_length());
        }

        const auto values = mpi::block_scatter(std::span{send_buffer}, values_per_rank, root,
                                               inter_communicator.get_inter_communicator());
        const auto dimensions = get_dimensions(blocks_to_receive);

        return {values, dimensions};
    }

}// namespace reshuffle::internal

#endif//MPI_COMM_UTILS_HPP
