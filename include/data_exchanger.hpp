#ifndef RESHUFFLE_DATA_EXCHANGER_HPP
#define RESHUFFLE_DATA_EXCHANGER_HPP

#include "concepts.hpp"
#include "grid_overlay.hpp"
#include "mpi_comm_utils.hpp"
#include "rank_information.hpp"

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
        const auto coordinate_owners_target = grid_overlay.get_coordinates_owners_target_grid();
        const auto coordinate_owners_origin = grid_overlay.get_coordinates_owners_origin_grid();

        for (int i = 0; i < multidimensional_blocks.size(); ++i) {
            const auto owner_origin_grid = coordinate_owners_origin[i];
            const auto owner_target_grid = coordinate_owners_target[i];

            // The owners in the send_blocks are relative to the final grid
            if (owner_origin_grid == rank_initial_grid) {
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
