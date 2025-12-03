#ifndef RESHUFFLE_GRID_OVERLAY_HPP
#define RESHUFFLE_GRID_OVERLAY_HPP

#include "block.hpp"
#include "block_overlay.hpp"
#include "coordinates.hpp"
#include "grid_layout.hpp"
#include "multidimensional_block.hpp"
#include "multidimensional_overlay.hpp"

#include <array>


namespace reshuffle::internal {
    template<std::size_t N>
    class GridOverlayDev {
    public:
        GridOverlayDev(const GridLayout<N> &origin, const GridLayout<N> &target);

        [[nodiscard]] auto get_coordinates_owners_origin_grid() const
                -> std::vector<Coordinates<N>>;
        [[nodiscard]] auto get_coordinates_owners_target_grid() const
                -> std::vector<Coordinates<N>>;
        [[nodiscard]] auto get_multidimensional_blocks_origin() const
                -> std::vector<MultidimensionalBlock<N>>;
        [[nodiscard]] auto get_multidimensional_blocks_target() const
                -> std::vector<MultidimensionalBlock<N>>;

    private:
        std::array<std::vector<BlockOverlay>, N> _blocks_overlay;
    };

    template<std::size_t N>
    GridOverlayDev<N>::GridOverlayDev(const GridLayout<N> &origin, const GridLayout<N> &target)
        : _blocks_overlay(get_blocks_overlay(origin.get_blocks(), target.get_blocks())) {}

    template<std::size_t N>
    auto GridOverlayDev<N>::get_coordinates_owners_origin_grid() const
            -> std::vector<Coordinates<N>> {
        auto result = std::vector<Coordinates<N>>{};

        for (const auto cartesian_product = get_cartesian_product(_blocks_overlay);
             const auto &multidimensional_overlay: cartesian_product) {
            result.emplace_back(get_coordinates_origin(multidimensional_overlay));
        }

        return result;
    }

    template<std::size_t N>
    auto GridOverlayDev<N>::get_coordinates_owners_target_grid() const
            -> std::vector<Coordinates<N>> {

        auto result = std::vector<Coordinates<N>>{};

        for (const auto cartesian_product = get_cartesian_product(_blocks_overlay);
             const auto &multidimensional_overlay: cartesian_product) {
            result.emplace_back(get_coordinates_target(multidimensional_overlay));
        }

        return result;
    }

    template<std::size_t N>
    auto GridOverlayDev<N>::get_multidimensional_blocks_origin() const
            -> std::vector<MultidimensionalBlock<N>> {
        auto result = std::vector<MultidimensionalBlock<N>>{};

        for (const auto cartesian_product = get_cartesian_product(_blocks_overlay);
             const auto &multidimensional_overlay: cartesian_product) {
            result.emplace_back(get_multidimensional_block_origin(multidimensional_overlay));
        }

        return result;
    }

    template<std::size_t N>
    auto GridOverlayDev<N>::get_multidimensional_blocks_target() const
            -> std::vector<MultidimensionalBlock<N>> {
        auto result = std::vector<MultidimensionalBlock<N>>{};

        for (const auto cartesian_product = get_cartesian_product(_blocks_overlay);
             const auto &multidimensional_overlay: cartesian_product) {
            result.emplace_back(get_multidimensional_block_target(multidimensional_overlay));
        }

        return result;
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_GRID_OVERLAY_HPP
