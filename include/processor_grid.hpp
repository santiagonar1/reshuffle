#ifndef PROCESSOR_GRID_HPP
#define PROCESSOR_GRID_HPP

#include "dimensions.hpp"

namespace reshuffle::dev {
    template<std::size_t N>
    class ProcessorGrid {
    public:
        explicit ProcessorGrid(const Dimensions<N> &dimensions);

        [[nodiscard]] auto get_num_processors() const -> int;

        auto operator==(const ProcessorGrid &other) const -> bool;

    private:
        const int _num_processors;
        const Dimensions<N> _dimensions;
    };


    template<std::size_t N>
    ProcessorGrid<N>::ProcessorGrid(const Dimensions<N> &dimensions)
        : _dimensions(dimensions), _num_processors(calc_total_num_values(dimensions)) {}

    template<std::size_t N>
    auto ProcessorGrid<N>::get_num_processors() const -> int {
        return _num_processors;
    }

    template<std::size_t N>
    auto ProcessorGrid<N>::operator==(const ProcessorGrid<N> &other) const -> bool {
        return _num_processors == other._num_processors and _dimensions == other._dimensions;
    }
}// namespace reshuffle::dev

#endif//PROCESSOR_GRID_HPP
