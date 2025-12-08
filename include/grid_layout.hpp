#ifndef GRID_LAYOUT_HPP
#define GRID_LAYOUT_HPP

#include "block.hpp"
#include "cartesian_product.hpp"
#include "coordinates.hpp"
#include "multidimensional_block.hpp"
#include "processor_grid.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

#include <algorithm>
#include <ranges>
#include <vector>


namespace reshuffle::internal {
    template<std::size_t N>
    class GridLayout {
    public:
        explicit GridLayout(std::array<std::vector<Block>, N> blocks);

        [[nodiscard]] auto get_blocks() const -> const std::array<std::vector<Block>, N> &;
        [[nodiscard]] auto get_block_owner(const Coordinates<N> &block_coordinates,
                                           const ProcessorGrid<N> &processor_grid) const -> RankId;
        [[nodiscard]] auto get_local_grid(RankId rank, const ProcessorGrid<N> &processor_grid) const
                -> GridLayout;
        [[nodiscard]] auto get_multidimensional_blocks() const
                -> const std::vector<MultidimensionalBlock<N>> &;

        [[nodiscard]] auto operator==(const GridLayout &other) const -> bool;

    private:
        [[nodiscard]] auto get_processor_coordinates(const Coordinates<N> &block_coordinates) const
                -> Coordinates<N>;


        // Here the order matters, as _multidimensional_blocks needs to be initialized before _blocks.
        const std::vector<MultidimensionalBlock<N>> _multidimensional_blocks;
        const std::array<std::vector<Block>, N> _blocks;
    };

    template<std::size_t N>
    GridLayout<N>::GridLayout(std::array<std::vector<Block>, N> blocks)
        : _multidimensional_blocks(get_cartesian_product(blocks)), _blocks(std::move(blocks)) {}

    template<std::size_t N>
    auto GridLayout<N>::get_blocks() const -> const std::array<std::vector<Block>, N> & {
        return _blocks;
    }

    template<std::size_t N>
    auto GridLayout<N>::get_block_owner(const Coordinates<N> &block_coordinates,
                                        const ProcessorGrid<N> &processor_grid) const -> RankId {
        const auto processor_coordinates = get_processor_coordinates(block_coordinates);
        return map_indices(processor_coordinates, processor_grid.get_dimensions()).value();
    }

    template<std::size_t N>
    auto GridLayout<N>::get_local_grid(const RankId rank,
                                       const ProcessorGrid<N> &processor_grid) const -> GridLayout {

        const auto filter_fn = [rank, &processor_grid](const auto &block_tuple) {
            const auto owner_coordinates = get_owner_coordinates(block_tuple);
            return processor_grid.get_processor_id(owner_coordinates) == rank;
        };

        // Create a vector to store filtered results -- vector of arrays
        auto local_blocks_view = _multidimensional_blocks | std::views::filter(filter_fn);
        const auto local_multidimensional_blocks = std::vector<MultidimensionalBlock<N>>{
                local_blocks_view.begin(), local_blocks_view.end()};

        // From now on, array of vectors
        auto local_blocks = internal::unzip(local_multidimensional_blocks);

        std::ranges::transform(local_blocks, local_blocks.begin(), [](const auto &block_vector) {
            return make_contiguous(remove_duplicates(block_vector));
        });

        return GridLayout{std::move(local_blocks)};
    }

    template<std::size_t N>
    auto GridLayout<N>::get_multidimensional_blocks() const
            -> const std::vector<MultidimensionalBlock<N>> & {
        return _multidimensional_blocks;
    }

    template<std::size_t N>
    auto GridLayout<N>::get_processor_coordinates(const Coordinates<N> &block_coordinates) const
            -> Coordinates<N> {
        auto processor_coordinates = Coordinates<N>{};
        for (int dim = 0; dim < N; ++dim) {
            if (block_coordinates[dim] < 0 or block_coordinates[dim] >= _blocks[dim].size()) {
                throw std::out_of_range("block_coordinates out of range");
            }
            processor_coordinates[dim] = _blocks[dim][block_coordinates[dim]].get_owner();
        }
        return processor_coordinates;
    }

    template<std::size_t N>
    auto GridLayout<N>::operator==(const GridLayout &other) const -> bool {
        return _multidimensional_blocks == other._multidimensional_blocks;
    }

}// namespace reshuffle::internal

#endif//GRID_LAYOUT_HPP
