#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <mpi.h>
#include <ranges>
#include <vector>

#include "communication_package.hpp"
#include "concepts.hpp"
#include "context.hpp"
#include "dimensions.hpp"
#include "greedy_rank_order_strategy.hpp"
#include "hungarian_rank_order_strategy.hpp"
#include "mpi_comm_utils.hpp"
#include "mpi_utils.hpp"
#include "profiler.hpp"
#include "rank_information.hpp"
#include "rank_order.hpp"
#include "utils.hpp"


namespace reshuffle {
    namespace internal {
        // TODO: I think this function should return a pair of vectors of MultiBlock
        // I have not changed it yet to avoid having to modify the exchange, but that should
        // be my next modification
        template<std::size_t N>
        auto get_send_and_receive_blocks(const GridOverlay<N> &grid_overlay,
                                         const Coordinates<N> &rank_initial_grid,
                                         const Coordinates<N> &rank_final_grid)
                -> std::pair<std::array<std::vector<Block>, N>, std::array<std::vector<Block>, N>> {
            PROFILE_SCOPE_NAMED("get_send_and_receive_blocks");

            auto send_blocks = std::array<std::vector<Block>, N>{};
            auto receive_blocks = std::array<std::vector<Block>, N>{};

            const auto &multidimensional_blocks =
                    grid_overlay.get_grid().get_multidimensional_blocks();
            const auto &coordinate_owners_target =
                    grid_overlay.get_coordinates_owners_target_grid();

            for (int i = 0; i < multidimensional_blocks.size(); ++i) {
                const auto owner_initial_grid = get_owner_coordinates(multidimensional_blocks[i]);
                const auto &owner_target_grid = coordinate_owners_target[i];

                // The owners in the send_blocks are relative to the final grid
                if (owner_initial_grid == rank_initial_grid) {
                    for (int dim = 0; dim < N; ++dim) {
                        const auto &block = multidimensional_blocks[i][dim];
                        send_blocks[dim].emplace_back(block.get_interval(), owner_target_grid[dim]);
                    }
                }

                // The owners in the receive_blocks are relative to the initial grid
                if (owner_target_grid == rank_final_grid) {
                    for (int dim = 0; dim < N; ++dim) {
                        const auto &block = multidimensional_blocks[i][dim];
                        receive_blocks[dim].emplace_back(block);
                    }
                }
            }

            std::ranges::transform(send_blocks, send_blocks.begin(), [](const auto &block_vector) {
                return join(remove_duplicates(block_vector));
            });


            std::ranges::transform(
                    receive_blocks, receive_blocks.begin(),
                    [](const auto &block_vector) { return join(remove_duplicates(block_vector)); });


            return {send_blocks, receive_blocks};
        }
    }// namespace internal

    template<typename T, typename Extents>
        requires concepts::FundamentalType<T> ||
                 concepts::Serializable<T>
                 auto shuffle(std::mdspan<const T, Extents> local_values,
                              const Context<Extents::rank()> &initial_context,
                              const Context<Extents::rank()> &final_context)
                         -> std::pair<std::vector<T>, Dimensions<Extents::rank()>>
                     requires(Extents::rank() <= 3)
    {
        PROFILE_SCOPE_NAMED("Shuffle");
        if (initial_context == final_context) {
            return {internal::get_1D_data(local_values), internal::get_dimensions(local_values)};
        }

        const auto grid_overlay = initial_context.distribution.get_grid_layout().get_overlay(
                final_context.distribution.get_grid_layout(),
                final_context.distribution.get_processor_grid());

        // TODO: Deal with partially disjoint communicators
        const auto inter_communicator =
                internal::InterCommunicator(initial_context.comm, final_context.comm);

        const auto initial_processor_grid = initial_context.distribution.get_processor_grid();
        const auto final_processor_grid = final_context.distribution.get_processor_grid();

        const auto rank_information = internal::RankInformation{
                inter_communicator, initial_context.distribution.get_processor_grid(),
                final_context.distribution.get_processor_grid()};

        const auto [blocks_to_send, blocks_to_receive] = internal::get_send_and_receive_blocks(
                grid_overlay, rank_information.get_initial_rank_coordinates(),
                rank_information.get_final_rank_coordinates());

        if (initial_context.distribution.get_processor_grid().get_num_processors() == 1 and
            is_block_wise_distribution(final_context.distribution)) {
            const auto &multidimensional_blocks =
                    grid_overlay.get_grid().get_multidimensional_blocks();
            const auto root_coordinates = get_owner_coordinates(multidimensional_blocks[0]);
            const auto root_rank_initial_comm =
                    initial_processor_grid.get_processor_id(root_coordinates);
            const auto root_inter_comm = inter_communicator.get_inter_comm_rank(
                    root_rank_initial_comm,
                    internal::InterCommunicator::SelectCommunicator::INITIAL_COMM);
            return internal::scatter_values(local_values, blocks_to_send, blocks_to_receive,
                                            final_processor_grid, root_inter_comm,
                                            inter_communicator);
        }

        return internal::exchange_values(local_values, blocks_to_send, blocks_to_receive,
                                         initial_processor_grid, final_processor_grid,
                                         inter_communicator);
    }

    template<typename T>
    auto shuffle(const std::vector<std::vector<T>> &local_values, const Context<2> &initial_context,
                 const Context<2> &final_context) -> std::pair<std::vector<T>, Dimensions<2>> {
        auto flat_data = internal::to_vector(local_values);
        const auto dimensions = internal::get_dimensions(local_values);
        return shuffle(std::mdspan(std::as_const(flat_data).data(), dimensions), initial_context,
                       final_context);
    }

    template<std::size_t N>
    auto get_optimal_communicator(const Context<N> &initial_context,
                                  const Context<N> &final_context)
            -> std::optional<std::pair<MPI_Comm, std::vector<RankId>>> {
        const auto commWeight =
                internal::RankOrder<N>(initial_context.distribution, final_context.distribution,
                                       internal::HungarianRankOrderStrategy{});
        const auto reordering = commWeight.get_optimal_rank_order();
        return std::make_optional(std::make_pair(
                internal::RankOrder<N>::get_reordered_comm(final_context.comm, reordering),
                reordering));
    }

    template<std::size_t N>
    auto get_optimal_communicator_greedy(const Context<N> &initial_context,
                                         const Context<N> &final_context)
            -> std::optional<std::pair<MPI_Comm, std::vector<RankId>>> {
        const auto commWeight =
                internal::RankOrder<N>(initial_context.distribution, final_context.distribution,
                                       internal::GreedyRankOrderStrategy{});
        const auto reordering = commWeight.get_optimal_rank_order();
        return std::make_optional(std::make_pair(
                internal::RankOrder<N>::get_reordered_comm(final_context.comm, reordering),
                reordering));
    }
}// namespace reshuffle


#endif//RESHUFFLE_SHUFFLE_HPP
