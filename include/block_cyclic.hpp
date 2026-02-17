#ifndef RESHUFFLE_BLOCK_CYCLIC_HPP
#define RESHUFFLE_BLOCK_CYCLIC_HPP

#include "block.hpp"
#include "data_distribution.hpp"
#include "grid_layout.hpp"
#include "processor_grid.hpp"
#include "profiler.hpp"

#include <cmath>
#include <vector>


namespace reshuffle::distribution {

    namespace internal {
        [[nodiscard]] auto create_blocks(int num_values, int block_size, int num_processors)
                -> std::vector<Block>;

        template<std::size_t N>
        [[nodiscard]] auto create_blocks(const Dimensions<N> &num_values,
                                         const Dimensions<N> &block_size,
                                         const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N>;
    }// namespace internal

    template<std::size_t N>
    class BlockCyclic final : public DataDistribution<N> {
    public:
        BlockCyclic(const Dimensions<N> &num_global_values, const Dimensions<N> &block_sizes,
                    const ProcessorGrid<N> &processor_grid);

        [[nodiscard]] auto get_grid_layout() const -> const GridLayout<N> & override;
        [[nodiscard]] auto get_processor_grid() const -> const ProcessorGrid<N> & override;
        [[nodiscard]] auto get_num_blocks_per_dimension() const -> Dimensions<N>;
        [[nodiscard]] auto is_block_wise() const -> bool override;
        [[nodiscard]] auto clone() const -> std::unique_ptr<DataDistribution<N>> override;

    private:
        const Dimensions<N> _num_global_values;
        const Dimensions<N> _block_sizes;
        const ProcessorGrid<N> _processor_grid;
        const GridLayout<N> _grid_layout;
    };

    template<std::size_t N>
    BlockCyclic<N>::BlockCyclic(const Dimensions<N> &num_global_values,
                                const Dimensions<N> &block_sizes,
                                const ProcessorGrid<N> &processor_grid)
        : _num_global_values(num_global_values), _block_sizes(block_sizes),
          _processor_grid(processor_grid),
          _grid_layout({internal::create_blocks(num_global_values, block_sizes, processor_grid)}) {}

    template<std::size_t N>
    auto BlockCyclic<N>::get_grid_layout() const -> const GridLayout<N> & {
        return _grid_layout;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::get_processor_grid() const -> const ProcessorGrid<N> & {
        return _processor_grid;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::is_block_wise() const -> bool {
        const auto num_processors_per_dimension = _processor_grid.get_dimensions();
        const auto num_blocks_per_dimension = get_num_blocks_per_dimension();
        for (int dim = 0; dim < N; ++dim) {
            if (num_blocks_per_dimension[dim] != num_processors_per_dimension[dim] and
                num_processors_per_dimension[dim] != 1) {
                return false;
            }
        }
        return true;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::get_num_blocks_per_dimension() const -> Dimensions<N> {
        auto num_blocks_per_dimension = Dimensions<N>{};
        for (int i = 0; i < N; ++i) {
            const auto num_blocks =
                    std::ceil(static_cast<double>(_num_global_values[i]) / _block_sizes[i]);
            num_blocks_per_dimension[i] = static_cast<int>(num_blocks);
        }
        return num_blocks_per_dimension;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::clone() const -> std::unique_ptr<DataDistribution<N>> {
        return std::make_unique<BlockCyclic>(*this);
    }

    namespace internal {
        template<std::size_t N>
        auto create_blocks(const Dimensions<N> &num_values, const Dimensions<N> &block_size,
                           const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N> {
            PROFILE_SCOPE_NAMED("create_blocks_nd");
            auto blocks = std::array<std::vector<Block>, N>{};
            const auto processor_dimensions = processor_grid.get_dimensions();
            for (int i = 0; i < N; ++i) {
                blocks[i] = create_blocks(num_values[i], block_size[i], processor_dimensions[i]);
            }
            return blocks;
        }
    }// namespace internal
}// namespace reshuffle::distribution


#endif//RESHUFFLE_BLOCK_CYCLIC_HPP
