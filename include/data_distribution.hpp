#ifndef RESHUFFLE_DATA_DISTRIBUTION_HPP
#define RESHUFFLE_DATA_DISTRIBUTION_HPP

#include "grid_layout.hpp"
#include "processor_grid.hpp"

#include <memory>

namespace reshuffle::distribution {

    template<std::size_t N>
    class DataDistribution {
    public:
        virtual ~DataDistribution() = default;

        [[nodiscard]] virtual auto get_grid_layout() const -> const GridLayout<N> & = 0;
        [[nodiscard]] virtual auto get_processor_grid() const -> const ProcessorGrid<N> & = 0;
        [[nodiscard]] virtual auto is_block_wise() const -> bool = 0;
        [[nodiscard]] virtual auto clone() const -> std::unique_ptr<DataDistribution> = 0;

        [[nodiscard]] auto operator==(const DataDistribution &other) const -> bool;
    };

    template<std::size_t N>
    auto DataDistribution<N>::operator==(const DataDistribution &other) const -> bool {
        return get_grid_layout() == other.get_grid_layout() and
               get_processor_grid() == other.get_processor_grid();
    }

}// namespace reshuffle::distribution

#endif//RESHUFFLE_DATA_DISTRIBUTION_HPP
