#ifndef RESHUFFLE_BLOCK_WISE_HPP
#define RESHUFFLE_BLOCK_WISE_HPP

#include "data_distribution.hpp"
#include "dimensions.hpp"
#include "processor_grid.hpp"

namespace reshuffle {

    namespace internal {
        [[nodiscard]] auto create_evenly_blocks(int num_values, int num_processors)
                -> std::vector<Block>;

        template<std::size_t N>
        [[nodiscard]] auto create_evenly_blocks(const Dimensions<N> &num_values,
                                                const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N>;
    }// namespace internal

    template<std::size_t N>
    class BlockWise final : public DataDistribution<N> {
    public:
        BlockWise(const Dimensions<N> &num_global_values, const ProcessorGrid<N> &processor_grid);

        [[nodiscard]] auto get_grid_layout() const -> const internal::GridLayout<N> & override;
        [[nodiscard]] auto get_processor_grid() const -> const ProcessorGrid<N> & override;
        [[nodiscard]] auto get_num_blocks_per_dimension() const -> Dimensions<N>;
        [[nodiscard]] auto is_block_wise() const -> bool override;
        [[nodiscard]] auto clone() const -> std::unique_ptr<DataDistribution<N>> override;

    private:
        const Dimensions<N> _num_global_values;
        const ProcessorGrid<N> _processor_grid;
        const internal::GridLayout<N> _grid_layout;
    };

    template<std::size_t N>
    BlockWise<N>::BlockWise(const Dimensions<N> &num_global_values,
                            const ProcessorGrid<N> &processor_grid)
        : _num_global_values(num_global_values), _processor_grid(processor_grid),
          _grid_layout(internal::create_evenly_blocks(num_global_values, processor_grid)) {}

    template<std::size_t N>
    auto BlockWise<N>::get_grid_layout() const -> const internal::GridLayout<N> & {
        return _grid_layout;
    }

    template<std::size_t N>
    auto BlockWise<N>::get_processor_grid() const -> const ProcessorGrid<N> & {
        return _processor_grid;
    }

    template<std::size_t N>
    auto BlockWise<N>::get_num_blocks_per_dimension() const -> Dimensions<N> {
        // For BlockWise, we should have as many blocks as processes
        return _processor_grid.get_dimensions();
    }

    template<std::size_t N>
    auto BlockWise<N>::is_block_wise() const -> bool {
        return true;
    }

    template<std::size_t N>
    auto BlockWise<N>::clone() const -> std::unique_ptr<DataDistribution<N>> {
        return std::make_unique<BlockWise>(*this);
    }

    namespace internal {
        template<std::size_t N>
        auto create_evenly_blocks(const Dimensions<N> &num_values,
                                  const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N> {
            auto blocks = std::array<std::vector<Block>, N>{};
            const auto processor_dimensions = processor_grid.get_dimensions();
            for (int i = 0; i < N; ++i) {
                blocks[i] = create_evenly_blocks(num_values[i], processor_dimensions[i]);
            }
            return blocks;
        }
    }// namespace internal
}// namespace reshuffle

#endif//RESHUFFLE_BLOCK_WISE_HPP
