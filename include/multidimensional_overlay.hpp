#ifndef RESHUFFLE_MULTIDIMENSIONAL_OVERLAY_HPP
#define RESHUFFLE_MULTIDIMENSIONAL_OVERLAY_HPP

#include "block_overlay.hpp"
#include "coordinates.hpp"
#include "multidimensional_block.hpp"

#include <array>

namespace reshuffle::internal {
    template<std::size_t N>
    using MultidimensionalOverlay = std::array<BlockOverlay, N>;

    template<std::size_t N>
    [[nodiscard]] auto
    get_coordinates_origin(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> Coordinates<N>;

    template<std::size_t N>
    [[nodiscard]] auto
    get_coordinates_target(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> Coordinates<N>;

    template<std::size_t N>
    [[nodiscard]] auto
    get_multidimensional_block_origin(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> MultidimensionalBlock<N>;

    template<std::size_t N>
    [[nodiscard]] auto
    get_multidimensional_block_target(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> MultidimensionalBlock<N>;


    template<std::size_t N>
    auto get_coordinates_origin(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> Coordinates<N> {
        auto coordinates = Coordinates<N>{};
        for (int dim = 0; dim < N; ++dim) {
            coordinates[dim] = multidimensional_overlay[dim].id_origin;
        }
        return coordinates;
    }

    template<std::size_t N>
    auto get_coordinates_target(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> Coordinates<N> {
        auto coordinates = Coordinates<N>{};
        for (int dim = 0; dim < N; ++dim) {
            coordinates[dim] = multidimensional_overlay[dim].id_target;
        }
        return coordinates;
    }

    template<std::size_t N>
    auto
    get_multidimensional_block_origin(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> MultidimensionalBlock<N> {
        auto multidimensional_block = MultidimensionalBlock<N>{};
        for (int dim = 0; dim < N; ++dim) {
            multidimensional_block[dim] = Block{multidimensional_overlay[dim].interval,
                                                multidimensional_overlay[dim].id_origin};
        }
        return multidimensional_block;
    }

    template<std::size_t N>
    auto
    get_multidimensional_block_target(const MultidimensionalOverlay<N> &multidimensional_overlay)
            -> MultidimensionalBlock<N> {
        auto multidimensional_block = MultidimensionalBlock<N>{};
        for (int dim = 0; dim < N; ++dim) {
            multidimensional_block[dim] = Block{multidimensional_overlay[dim].interval,
                                                multidimensional_overlay[dim].id_target};
        }
        return multidimensional_block;
    }

}// namespace reshuffle::internal

#endif//RESHUFFLE_MULTIDIMENSIONAL_OVERLAY_HPP
