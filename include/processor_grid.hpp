#ifndef PROCESSOR_GRID_HPP
#define PROCESSOR_GRID_HPP

#include "concepts.hpp"
#include "coordinates.hpp"
#include "dimensions.hpp"
#include "rank_id.hpp"

namespace reshuffle {
    template<std::size_t N>
    class ProcessorGrid {
    public:
        explicit ProcessorGrid(const Dimensions<N> &dimensions);

        template<concepts::Int... Ts>
            requires(sizeof...(Ts) == N)
        explicit ProcessorGrid(Ts &&...xs);

        [[nodiscard]] auto get_num_processors() const -> int;
        [[nodiscard]] auto get_dimensions() const -> Dimensions<N>;
        [[nodiscard]] auto get_processor_id(const Coordinates<N> &coordinates) const -> RankId;
        [[nodiscard]] auto get_processor_coordinates(RankId processor) const -> Coordinates<N>;

        auto operator==(const ProcessorGrid &other) const -> bool;

    private:
        const int _num_processors;
        const Dimensions<N> _dimensions;
    };


    template<std::size_t N>
    ProcessorGrid<N>::ProcessorGrid(const Dimensions<N> &dimensions)
        : _num_processors(internal::calc_total_num_values(dimensions)), _dimensions(dimensions) {}

    template<std::size_t N>
    template<concepts::Int... Ts>
        requires(sizeof...(Ts) == N)
    ProcessorGrid<N>::ProcessorGrid(Ts &&...xs)
        : ProcessorGrid{Dimensions<N>{std::forward<Ts>(xs)...}} {}

    template<std::size_t N>
    auto ProcessorGrid<N>::get_num_processors() const -> int {
        return _num_processors;
    }

    template<std::size_t N>
    auto ProcessorGrid<N>::get_dimensions() const -> Dimensions<N> {
        return _dimensions;
    }

    template<std::size_t N>
    auto ProcessorGrid<N>::get_processor_id(const Coordinates<N> &coordinates) const -> RankId {
        return internal::map_indices(coordinates, _dimensions).value_or(INVALID_RANK_ID);
    }

    template<std::size_t N>
    auto ProcessorGrid<N>::get_processor_coordinates(RankId processor) const -> Coordinates<N> {
        return internal::map_index(processor, _dimensions)
                .value_or(internal::get_invalid_coordinates<N>());
    }


    template<std::size_t N>
    auto ProcessorGrid<N>::operator==(const ProcessorGrid<N> &other) const -> bool {
        return _num_processors == other._num_processors and _dimensions == other._dimensions;
    }

    // Deduction guide
    template<concepts::Int... Ts>
    ProcessorGrid(Ts &&...xs) -> ProcessorGrid<sizeof...(Ts)>;

}// namespace reshuffle

#endif//PROCESSOR_GRID_HPP
