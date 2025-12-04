#ifndef MULTIDIMENSIONAL_BLOCK_HPP
#define MULTIDIMENSIONAL_BLOCK_HPP

#include "block.hpp"
#include "coordinates.hpp"
#include "profiler.hpp"

#include <format>

namespace reshuffle::internal {
    template<std::size_t N>
    using MultidimensionalBlock = std::array<Block, N>;

    template<std::size_t N>
    [[nodiscard]] auto get_owner_coordinates(const MultidimensionalBlock<N> &multidimensional_block)
            -> Coordinates<N> {
        PROFILE_SCOPE_NAMED("get_owner_coordinates");
        auto owner_coordinates = Coordinates<N>{};
        for (int i = 0; i < N; ++i) {
            owner_coordinates[i] = multidimensional_block[i].get_owner();
        }
        return owner_coordinates;
    }

    template<std::size_t N>
    [[nodiscard]] auto get_num_elements(const MultidimensionalBlock<N> &multidimensional_block)
            -> int {
        PROFILE_SCOPE_NAMED("get_num_elements");
        auto num_elements = 1;
        for (int i = 0; i < N; ++i) {
            num_elements *= multidimensional_block[i].get_num_elements();
        }
        return num_elements;
    }

    template<std::size_t N>
    [[nodiscard]] auto get_num_elements(const std::vector<MultidimensionalBlock<N>> &blocks)
            -> int {
        return std::accumulate(blocks.cbegin(), blocks.cend(), 0, [](int sum, const auto &block) {
            return sum + get_num_elements(block);
        });
    }

    template<std::size_t N>
    [[nodiscard]] auto replace_block(const MultidimensionalBlock<N> &multidimensional_block,
                                     const Block &block, int dim) -> MultidimensionalBlock<N> {

        if (dim < 0 or dim >= N) {
            const auto err_msg = std::format("dim must be in [0, {}), but got {}", N, dim);
            throw std::invalid_argument(err_msg);
        }

        auto new_multidimensional_block = multidimensional_block;
        new_multidimensional_block[dim] = block;
        return new_multidimensional_block;
    }

    template<std::size_t N>
    [[nodiscard]] auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks, int dim)
            -> std::vector<MultidimensionalBlock<N>> {

        if (dim < 0 or dim >= N) {
            const auto err_msg = std::format("dim must be in [0, {}), but got {}", N, dim);
            throw std::invalid_argument(err_msg);
        }

        if (blocks.empty()) { return {}; }

        const auto first_block = blocks.front();
        const auto num_elements_first_block = first_block[dim].get_num_elements();

        auto contiguous_blocks = std::vector<MultidimensionalBlock<N>>{};
        contiguous_blocks.emplace_back(replace_block(
                first_block, Block{{0, num_elements_first_block}, first_block[dim].get_owner()},
                dim));

        for (const auto &multidimensional_block: blocks | std::views::drop(1)) {
            const auto last_block = contiguous_blocks.back();
            const auto num_elements_current_block = multidimensional_block[dim].get_num_elements();
            contiguous_blocks.emplace_back(
                    replace_block(multidimensional_block,
                                  Block{{last_block[dim].get_interval().get_right_bound(),
                                         last_block[dim].get_interval().get_right_bound() +
                                                 num_elements_current_block},
                                        multidimensional_block[dim].get_owner()},
                                  dim));
        }

        return contiguous_blocks;
    }

    template<std::size_t N>
    [[nodiscard]] auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks)
            -> std::vector<MultidimensionalBlock<N>> {
        auto contiguous_blocks = blocks;
        for (int dim = 0; dim < N; ++dim) {
            contiguous_blocks = make_contiguous(contiguous_blocks, dim);
        }
        return contiguous_blocks;
    }

}// namespace reshuffle::internal

#endif//MULTIDIMENSIONAL_BLOCK_HPP
