#ifndef GRID_LAYOUT_HPP
#define GRID_LAYOUT_HPP

#include "block.hpp"
#include "cartesian_product.hpp"
#include "coordinates.hpp"
#include "multidimensional_block.hpp"
#include "processor_grid.hpp"
#include "rank_id.hpp"

#include <algorithm>
#include <ranges>
#include <vector>


namespace reshuffle::dev {
    template<std::size_t N>
    struct GridOverlay;

    template<std::size_t N>
    class GridLayout {
    public:
        explicit GridLayout(std::array<std::vector<Block>, N> blocks);

        [[nodiscard]] auto get_blocks() const -> const std::array<std::vector<Block>, N> &;
        [[nodiscard]] auto
        get_block_owner(const ::reshuffle::internal::Coordinates<N> &block_coordinates,
                        const ProcessorGrid<N> &processor_grid) const -> rank_id;
        [[nodiscard]] auto get_overlay(const GridLayout &target_grid,
                                       const ProcessorGrid<N> &target_processor_grid) const
                -> GridOverlay<N>;
        [[nodiscard]] auto get_local_grid(rank_id rank,
                                          const ProcessorGrid<N> &processor_grid) const
                -> GridLayout;
        [[nodiscard]] auto get_multidimensional_blocks() const
                -> const std::vector<MultidimensionalBlock<N>> &;

    private:
        [[nodiscard]] auto get_processor_coordinates(
                const ::reshuffle::internal::Coordinates<N> &block_coordinates) const
                -> ::reshuffle::internal::Coordinates<N>;


        // Here the order matters, as _multidimensional_blocks needs to be initialized before _blocks.
        const std::vector<MultidimensionalBlock<N>> _multidimensional_blocks;
        const std::array<std::vector<Block>, N> _blocks;
    };

    template<std::size_t N>
    struct GridOverlay {
        GridLayout<N> grid;
        std::array<std::vector<rank_id>, N> owners_target_grid;
    };

    template<std::size_t N>
    GridLayout<N>::GridLayout(std::array<std::vector<Block>, N> blocks)
        : _multidimensional_blocks(internal::get_cartesian_product(blocks)),
          _blocks(std::move(blocks)) {}

    template<std::size_t N>
    auto GridLayout<N>::get_blocks() const -> const std::array<std::vector<Block>, N> & {
        return _blocks;
    }

    template<std::size_t N>
    auto
    GridLayout<N>::get_block_owner(const ::reshuffle::internal::Coordinates<N> &block_coordinates,
                                   const ProcessorGrid<N> &processor_grid) const -> rank_id {
        const auto processor_coordinates = get_processor_coordinates(block_coordinates);
        return ::reshuffle::internal::map_indices(processor_coordinates,
                                                  processor_grid.get_dimensions());
    }


    inline auto get_overlay_imp(const GridLayout<1> &origin_grid, const GridLayout<1> &target_grid,
                                const int target_num_processors) -> GridOverlay<1> {

        const auto &origin_blocks = origin_grid.get_blocks()[0];
        const auto &target_blocks = target_grid.get_blocks()[0];

        if (target_blocks.front().get_interval().get_left_bound() !=
            origin_blocks.front().get_interval().get_left_bound()) {
            throw std::invalid_argument("target_grid does not start at same index as this grid");
        }

        if (target_blocks.back().get_interval().get_right_bound() !=
            origin_blocks.back().get_interval().get_right_bound()) {
            throw std::invalid_argument("target_grid does not end at same index as this grid");
        }

        auto sub_blocks = std::vector<Block>{};
        auto owners_target_grid = std::vector<rank_id>{};

        int pos_target_grid{};
        for (const auto &block: origin_blocks) {
            while (pos_target_grid < target_blocks.size()) {
                const auto block_overlay = block.get_overlay(target_blocks[pos_target_grid]);
                if (not block_overlay.has_value()) { break; }
                sub_blocks.emplace_back(block_overlay.value());
                owners_target_grid.emplace_back(target_grid.get_block_owner(
                        ::reshuffle::internal::Coordinates{pos_target_grid},
                        ProcessorGrid<1>{{target_num_processors}}));
                ++pos_target_grid;
            }

            const auto last_overlay = sub_blocks.back();
            const auto checked_all_target_grid_blocks = pos_target_grid == target_blocks.size();
            const auto there_are_missing_blocks =
                    last_overlay.get_interval().get_right_bound() !=
                    origin_blocks.back().get_interval().get_right_bound();
            const auto have_not_checked_all_blocks_but_missed_some =
                    not checked_all_target_grid_blocks and
                    last_overlay.get_interval().get_right_bound() <
                            target_blocks[pos_target_grid].get_interval().get_left_bound();

            if ((checked_all_target_grid_blocks and there_are_missing_blocks) or
                have_not_checked_all_blocks_but_missed_some) {
                --pos_target_grid;
            }
        }

        return GridOverlay{GridLayout{std::move(std::array{sub_blocks})},
                           std::move(std::array{owners_target_grid})};
    }

    template<std::size_t N>
    auto GridLayout<N>::get_overlay(const GridLayout &target_grid,
                                    const ProcessorGrid<N> &target_processor_grid) const
            -> GridOverlay<N> {

        auto pairs_per_dimension = std::views::zip(_blocks, target_grid._blocks,
                                                   target_processor_grid.get_dimensions());

        auto blocks_overlay = std::array<std::vector<Block>, N>{};
        auto owners_target_grid = std::array<std::vector<rank_id>, N>{};

        int i{};
        for (const auto &[blocks_origin, blocks_target, num_processors]: pairs_per_dimension) {
            const auto one_dim_overlay =
                    get_overlay_imp(GridLayout<1>{std::array{blocks_origin}},
                                    GridLayout<1>{std::array{blocks_target}}, num_processors);
            blocks_overlay[i] = one_dim_overlay.grid.get_blocks()[0];
            owners_target_grid[i] = one_dim_overlay.owners_target_grid[0];
            ++i;
        }


        return GridOverlay<N>{GridLayout<N>{std::move(blocks_overlay)},
                              std::move(owners_target_grid)};
    }

    //TODO: Move to utils and add tests
    template<typename Tuple>
    auto tuple_elements_to_vectors(const std::vector<Tuple> &tuples)
            -> std::array<std::vector<std::tuple_element_t<0, Tuple>>, std::tuple_size_v<Tuple>> {
        using FirstType = std::tuple_element_t<0, Tuple>;
        constexpr std::size_t tuple_size = std::tuple_size_v<Tuple>;
        auto result = std::array<std::vector<FirstType>, tuple_size>{};

        for (const auto &tuple: tuples) {
            std::apply(
                    [&result](const auto &...elements) {
                        std::size_t idx = 0;
                        ((result[idx++].push_back(elements)), ...);
                    },
                    tuple);
        }

        return result;
    }

    namespace internal {
        template<std::size_t N>
        auto unpack_multidimensional_blocks(
                const std::vector<MultidimensionalBlock<N>> &multidimensional_blocks)
                -> std::array<std::vector<Block>, N> {
            return tuple_elements_to_vectors(multidimensional_blocks);
        }
    }// namespace internal


    template<std::size_t N>
    auto GridLayout<N>::get_local_grid(const rank_id rank,
                                       const ProcessorGrid<N> &processor_grid) const -> GridLayout {

        const auto filter_fn = [rank, &processor_grid](const auto &block_tuple) {
            const auto owner_coordinates = std::apply(
                    [](const auto &...blocks) {
                        return ::reshuffle::internal::Coordinates{blocks.get_owner()...};
                    },
                    block_tuple);

            return processor_grid.get_processor_id(owner_coordinates) == rank;
        };

        // Create a vector to store filtered results -- vector of arrays
        auto local_blocks_view = _multidimensional_blocks | std::views::filter(filter_fn);
        const auto local_multidimensional_blocks = std::vector<MultidimensionalBlock<N>>{
                local_blocks_view.begin(), local_blocks_view.end()};

        // From now on, array of vectors
        auto local_blocks = internal::unpack_multidimensional_blocks(local_multidimensional_blocks);

        std::ranges::transform(local_blocks, local_blocks.begin(), [](const auto &block_vector) {
            auto result = block_vector;
            std::ranges::sort(result);
            auto [new_end, _] = std::ranges::unique(result);
            result.erase(new_end, result.end());
            return result;
        });


        std::ranges::transform(local_blocks, local_blocks.begin(),
                               [](const auto &block_vector) { return join(block_vector); });

        return GridLayout{std::move(local_blocks)};
    }

    template<std::size_t N>
    auto GridLayout<N>::get_multidimensional_blocks() const
            -> const std::vector<MultidimensionalBlock<N>> & {
        return _multidimensional_blocks;
    }

    template<std::size_t N>
    auto GridLayout<N>::get_processor_coordinates(
            const ::reshuffle::internal::Coordinates<N> &block_coordinates) const
            -> ::reshuffle::internal::Coordinates<N> {
        auto processor_coordinates = ::reshuffle::internal::Coordinates<N>{};
        // Note: we use the reversed block, because the first coordinate is relative to
        // to the last vector. For example, the block (x, y) belongs to processor (0, 1) if
        // the x-th block in the first dimension belongs to 1, and the y-th block in the second
        // dimension belongs to 0
        auto reversed_blocks = _blocks | std::views::reverse;
        for (int dim = 0; dim < N; ++dim) {
            if (block_coordinates[dim] < 0 or
                block_coordinates[dim] >= reversed_blocks[dim].size()) {
                throw std::out_of_range("block_coordinates out of range");
            }
            processor_coordinates[dim] = reversed_blocks[dim][block_coordinates[dim]].get_owner();
        }
        return processor_coordinates;
    }
}// namespace reshuffle::dev

#endif//GRID_LAYOUT_HPP
