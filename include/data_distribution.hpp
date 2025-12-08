#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "grid_layout.hpp"
#include "processor_grid.hpp"

namespace reshuffle {

    template<std::size_t N>
    class DataDistribution {
    public:
        virtual ~DataDistribution() = default;

        [[nodiscard]] virtual auto get_grid_layout() const -> const internal::GridLayout<N> & = 0;
        [[nodiscard]] virtual auto get_processor_grid() const -> const ProcessorGrid<N> & = 0;
    };
}// namespace reshuffle

#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
