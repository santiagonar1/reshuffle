#ifndef RESHUFFLE_DATA_EXCHANGER_HPP
#define RESHUFFLE_DATA_EXCHANGER_HPP

#include "concepts.hpp"
#include "grid_overlay.hpp"
#include "mpi_comm_utils.hpp"
#include "multidimensional_block.hpp"
#include "rank_information.hpp"

#include <ranges>

namespace reshuffle::internal {
    template<std::size_t N>
    auto get_send_and_receive_blocks(const GridOverlay<N> &grid_overlay,
                                     const Coordinates<N> &rank_initial_grid,
                                     const Coordinates<N> &rank_final_grid)
            -> std::pair<std::array<std::vector<Block>, N>, std::array<std::vector<Block>, N>>;


    template<std::size_t N>
    auto get_send_and_receive_blocks(const GridOverlay<N> &grid_overlay,
                                     const RankInformation<N> &rank_information)
            -> std::pair<std::array<std::vector<Block>, N>, std::array<std::vector<Block>, N>>;

    template<concepts::Exchangeable T, std::size_t N>
    class DataExchanger {
    public:
        virtual ~DataExchanger() = default;

        virtual auto exchange() const -> std::pair<std::vector<T>, Dimensions<N>> = 0;
    };

    template<std::size_t N>
    struct SendReceiveBlocks {
        std::vector<MultidimensionalBlock<N>> send_blocks;
        std::vector<MultidimensionalBlock<N>> receive_blocks;
    };

    enum class IntervalType { LOCAL, GLOBAL };

    template<std::size_t N>
    [[nodiscard]] auto get_send_and_receive_blocks_dev(const GridOverlay<N> &grid_overlay,
                                                       const Coordinates<N> &rank_initial_grid,
                                                       const Coordinates<N> &rank_final_grid,
                                                       IntervalType interval_type)
            -> SendReceiveBlocks<N>;

    template<std::size_t N>
    [[nodiscard]] auto get_send_and_receive_blocks_dev(const GridOverlay<N> &grid_overlay,
                                                       const RankInformation<N> &rank_information,
                                                       IntervalType interval_type)
            -> SendReceiveBlocks<N>;

    template<std::size_t N>
    auto get_send_and_receive_blocks_dev(const GridOverlay<N> &grid_overlay,
                                         const Coordinates<N> &rank_initial_grid,
                                         const Coordinates<N> &rank_final_grid,
                                         const IntervalType interval_type) -> SendReceiveBlocks<N> {
        PROFILE_SCOPE_NAMED("get_send_and_receive_blocks");

        auto send_blocks = std::vector<MultidimensionalBlock<N>>{};
        auto receive_blocks = std::vector<MultidimensionalBlock<N>>{};

        const auto blocks_initial = grid_overlay.get_multidimensional_blocks_origin();
        const auto blocks_final = grid_overlay.get_multidimensional_blocks_target();

        for (const auto &[block_initial, block_final]:
             std::views::zip(blocks_initial, blocks_final)) {
            const auto owner_initial_grid = get_owner_coordinates(block_initial);
            const auto owner_final_grid = get_owner_coordinates(block_final);
            // The owners in the send_blocks are relative to the final grid
            if (owner_initial_grid == rank_initial_grid) { send_blocks.emplace_back(block_final); }

            // The owners in the receive_blocks are relative to the initial grid
            if (owner_final_grid == rank_final_grid) { receive_blocks.emplace_back(block_initial); }
        }

        switch (interval_type) {
            case IntervalType::LOCAL:
                return {make_contiguous(send_blocks), make_contiguous(receive_blocks)};
            case IntervalType::GLOBAL:
                return {send_blocks, receive_blocks};
        }

        throw std::runtime_error("Invalid interval type");
    }

    template<std::size_t N>
    [[nodiscard]] auto get_send_and_receive_blocks_dev(const GridOverlay<N> &grid_overlay,
                                                       const RankInformation<N> &rank_information,
                                                       IntervalType interval_type)
            -> SendReceiveBlocks<N> {
        return get_send_and_receive_blocks_dev(
                grid_overlay, rank_information.get_initial_rank_coordinates(),
                rank_information.get_final_rank_coordinates(), interval_type);
    }

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

        const auto multidimensional_blocks = grid_overlay.get_multidimensional_blocks_origin();
        const auto coordinate_owners_final = grid_overlay.get_coordinates_owners_target_grid();
        const auto coordinate_owners_initial = grid_overlay.get_coordinates_owners_origin_grid();

        for (const auto &[multidimensional_block, owner_initial_grid, owner_final_grid]:
             std::views::zip(multidimensional_blocks, coordinate_owners_initial,
                             coordinate_owners_final)) {
            // The owners in the send_blocks are relative to the final grid
            if (owner_initial_grid == rank_initial_grid) {
                for (int dim = 0; dim < N; ++dim) {
                    const auto &block = multidimensional_block[dim];
                    send_blocks[dim].emplace_back(block.get_interval(), owner_final_grid[dim]);
                }
            }

            // The owners in the receive_blocks are relative to the initial grid
            if (owner_final_grid == rank_final_grid) {
                for (int dim = 0; dim < N; ++dim) {
                    const auto &block = multidimensional_block[dim];
                    receive_blocks[dim].emplace_back(block);
                }
            }
        }

        std::ranges::transform(send_blocks, send_blocks.begin(), [](const auto &block_vector) {
            return make_contiguous(remove_duplicates(block_vector));
        });


        std::ranges::transform(receive_blocks, receive_blocks.begin(),
                               [](const auto &block_vector) {
                                   return make_contiguous(remove_duplicates(block_vector));
                               });


        return {send_blocks, receive_blocks};
    }

    template<std::size_t N>
    auto get_send_and_receive_blocks(const GridOverlay<N> &grid_overlay,
                                     const RankInformation<N> &rank_information)
            -> std::pair<std::array<std::vector<Block>, N>, std::array<std::vector<Block>, N>> {
        return get_send_and_receive_blocks(grid_overlay,
                                           rank_information.get_initial_rank_coordinates(),
                                           rank_information.get_final_rank_coordinates());
    }


}// namespace reshuffle::internal

#endif//RESHUFFLE_DATA_EXCHANGER_HPP
