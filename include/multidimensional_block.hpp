#ifndef MULTIDIMENSIONAL_BLOCK_HPP
#define MULTIDIMENSIONAL_BLOCK_HPP

#include "block.hpp"
#include "cartesian_product.hpp"
#include "coordinates.hpp"
#include "dimensions.hpp"
#include "profiler.hpp"
#include "utils.hpp"

#include <algorithm>
#include <format>

namespace reshuffle::internal {
    template<std::size_t N>
    using MultidimensionalBlock = std::array<Block, N>;

    template<std::size_t N>
    [[nodiscard]] auto get_owner_coordinates(const MultidimensionalBlock<N> &multidimensional_block)
            -> Coordinates<N>;

    template<std::size_t N>
    [[nodiscard]] auto get_num_elements(const MultidimensionalBlock<N> &multidimensional_block)
            -> int;

    template<std::size_t N>
    [[nodiscard]] auto get_num_elements(const std::vector<MultidimensionalBlock<N>> &blocks) -> int;

    template<std::size_t N>
    [[nodiscard]] auto replace_block(const MultidimensionalBlock<N> &multidimensional_block,
                                     const Block &block, int dim) -> MultidimensionalBlock<N>;

    template<std::size_t N>
    [[nodiscard]] auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks, int dim)
            -> std::vector<MultidimensionalBlock<N>>;

    template<std::size_t N>
    [[nodiscard]] auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks)
            -> std::vector<MultidimensionalBlock<N>>;

    template<std::size_t N>
    [[nodiscard]] auto get_dimensions(const MultidimensionalBlock<N> &multidimensional_block)
            -> Dimensions<N>;

    template<std::size_t N>
    [[nodiscard]] auto get_dimensions(const std::vector<MultidimensionalBlock<N>> &blocks)
            -> Dimensions<N>;

    [[nodiscard]] auto get_dimensions(const std::vector<Block> &blocks) -> int;

    template<std::size_t N>
    auto get_owner_coordinates(const MultidimensionalBlock<N> &multidimensional_block)
            -> Coordinates<N> {
        PROFILE_SCOPE_NAMED("get_owner_coordinates");
        auto owner_coordinates = Coordinates<N>{};
        for (int i = 0; i < N; ++i) {
            owner_coordinates[i] = multidimensional_block[i].get_owner();
        }
        return owner_coordinates;
    }

    template<std::size_t N>
    auto get_num_elements(const MultidimensionalBlock<N> &multidimensional_block) -> int {
        PROFILE_SCOPE_NAMED("get_num_elements");
        auto num_elements = 1;
        for (int i = 0; i < N; ++i) {
            num_elements *= multidimensional_block[i].get_num_elements();
        }
        return num_elements;
    }

    template<std::size_t N>
    auto get_num_elements(const std::vector<MultidimensionalBlock<N>> &blocks) -> int {
        return std::accumulate(blocks.cbegin(), blocks.cend(), 0, [](int sum, const auto &block) {
            return sum + get_num_elements(block);
        });
    }

    template<std::size_t N>
    auto replace_block(const MultidimensionalBlock<N> &multidimensional_block, const Block &block,
                       int dim) -> MultidimensionalBlock<N> {

        if (dim < 0 or dim >= N) {
            const auto err_msg = std::format("dim must be in [0, {}), but got {}", N, dim);
            throw std::invalid_argument(err_msg);
        }

        auto new_multidimensional_block = multidimensional_block;
        new_multidimensional_block[dim] = block;
        return new_multidimensional_block;
    }

    template<std::size_t N>
    auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks, const int dim)
            -> std::vector<MultidimensionalBlock<N>> {

        if (dim < 0 or dim >= N) {
            const auto err_msg = std::format("dim must be in [0, {}), but got {}", N, dim);
            throw std::invalid_argument(err_msg);
        }

        if (blocks.empty()) { return {}; }

        auto unidimensional_blocks = std::array<std::vector<Block>, N>{};
        for (const auto &multidimensional_block: blocks) {
            for (int i = 0; i < N; ++i) {
                unidimensional_blocks[i].emplace_back(multidimensional_block[i]);
            }
        }

        unidimensional_blocks[dim] = make_contiguous(remove_duplicates(unidimensional_blocks[dim]));
        return get_cartesian_product(unidimensional_blocks);
    }

    template<std::size_t N>
    auto make_contiguous(const std::vector<MultidimensionalBlock<N>> &blocks)
            -> std::vector<MultidimensionalBlock<N>> {
        if (blocks.empty()) { return {}; }

        auto unidimensional_blocks = std::array<std::vector<Block>, N>{};
        for (const auto &multidimensional_block: blocks) {
            for (int i = 0; i < N; ++i) {
                unidimensional_blocks[i].emplace_back(multidimensional_block[i]);
            }
        }

        for (auto &unidimensional_block: unidimensional_blocks) {
            unidimensional_block = make_contiguous(remove_duplicates(unidimensional_block));
        }

        return get_cartesian_product(unidimensional_blocks);
    }

    template<std::size_t N>
    auto get_dimensions(const MultidimensionalBlock<N> &multidimensional_block) -> Dimensions<N> {

        auto dimensions = Dimensions<N>{};

        for (int dim = 0; dim < N; ++dim) {
            dimensions[dim] = multidimensional_block[dim].get_num_elements();
        }

        return dimensions;
    }

    template<std::size_t N>
    auto get_dimensions(const std::vector<MultidimensionalBlock<N>> &blocks) -> Dimensions<N> {
        if (blocks.empty()) { return {}; }

        auto dimensions = Dimensions<N>{};

        auto unidimensional_blocks = std::array<std::vector<Block>, N>{};
        for (const auto &multidimensional_block: blocks) {
            for (int i = 0; i < N; ++i) {
                unidimensional_blocks[i].emplace_back(multidimensional_block[i]);
            }
        }

        for (int dim = 0; dim < N; ++dim) {
            dimensions[dim] = get_dimensions(unidimensional_blocks[dim]);
        }

        return dimensions;
    }

}// namespace reshuffle::internal

#endif//MULTIDIMENSIONAL_BLOCK_HPP
