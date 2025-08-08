#ifndef RESHUFFLE_BLOCK_CYCLIC_HPP
#define RESHUFFLE_BLOCK_CYCLIC_HPP

#include "block.hpp"
#include "grid_layout.hpp"
#include "processor_grid.hpp"
#include "profiler.hpp"

#include <cmath>
#include <vector>


namespace reshuffle {

    namespace internal {
        auto create_blocks(int num_values, int block_size, int num_processors)
                -> std::vector<Block>;

        template<std::size_t N>
        auto create_blocks(const Dimensions<N> &num_values, const Dimensions<N> &block_size,
                           const ProcessorGrid<N> &processor_grid)
                -> std::array<std::vector<Block>, N> {
            PROFILE_SCOPE_NAMED("create_blocks_nd");
            std::array<std::vector<Block>, N> blocks{};
            const auto processor_dimensions = processor_grid.get_dimensions();
            for (int i = 0; i < N; ++i) {
                blocks[i] = create_blocks(num_values[i], block_size[i], processor_dimensions[i]);
            }
            return blocks;
        }
    }// namespace internal

    template<std::size_t N>
    class BlockCyclic {
    public:
        BlockCyclic(const Dimensions<N> &num_global_values, const Dimensions<N> &block_sizes,
                    const ProcessorGrid<N> &processor_grid);

        [[nodiscard]] auto get_grid_layout() const -> const internal::GridLayout<N> &;
        [[nodiscard]] auto get_num_global_values(int dimension) const -> int;
        [[nodiscard]] auto get_processor_grid() const -> const ProcessorGrid<N> &;
        [[nodiscard]] auto get_num_blocks_per_dimension() const -> Dimensions<N>;

        auto operator==(const BlockCyclic &other) const -> bool;

    private:
        const Dimensions<N> _num_global_values;
        const Dimensions<N> _block_sizes;
        const ProcessorGrid<N> _processor_grid;
        const internal::GridLayout<N> _grid_layout;
    };

    template<std::size_t N>
    BlockCyclic<N>::BlockCyclic(const Dimensions<N> &num_global_values,
                                const Dimensions<N> &block_sizes,
                                const ProcessorGrid<N> &processor_grid)
        : _num_global_values(num_global_values), _block_sizes(block_sizes),
          _processor_grid(processor_grid),
          _grid_layout({internal::create_blocks(num_global_values, block_sizes, processor_grid)}) {}

    template<std::size_t N>
    auto BlockCyclic<N>::get_grid_layout() const -> const internal::GridLayout<N> & {
        return _grid_layout;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::get_num_global_values(int dimension) const -> int {
        return _num_global_values[dimension];
    }

    template<std::size_t N>
    auto BlockCyclic<N>::get_processor_grid() const -> const ProcessorGrid<N> & {
        return _processor_grid;
    }

    template<std::size_t N>
    auto BlockCyclic<N>::operator==(const BlockCyclic &other) const -> bool {
        return _num_global_values == other._num_global_values and
               _block_sizes == other._block_sizes and _processor_grid == other._processor_grid;
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
    [[nodiscard]] auto make_block_wise_distribution(const Dimensions<N> &num_global_values,
                                                    const ProcessorGrid<N> &processor_grid)
            -> BlockCyclic<N> {
        auto block_sizes = Dimensions<N>{};
        for (int i = 0; i < N; ++i) {
            const auto num_processors = processor_grid.get_dimensions()[i];
            block_sizes[i] = std::ceil(static_cast<double>(num_global_values[i]) / num_processors);
        }
        return BlockCyclic{num_global_values, block_sizes, processor_grid};
    }

    template<std::size_t N>
    [[nodiscard]] auto is_block_wise_distribution(const BlockCyclic<N> &distribution) -> bool {
        const auto num_processors_per_dimension =
                distribution.get_processor_grid().get_dimensions();
        const auto num_blocks_per_dimension = distribution.get_num_blocks_per_dimension();
        for (int dim = 0; dim < N; ++dim) {
            if (num_blocks_per_dimension[dim] != num_processors_per_dimension[dim] and
                num_processors_per_dimension[dim] != 1) {
                return false;
            }
        }
        return true;
    }
}// namespace reshuffle


#endif//RESHUFFLE_BLOCK_CYCLIC_HPP
