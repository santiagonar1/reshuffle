#ifndef MULTIDIMENSIONAL_DATA_HPP
#define MULTIDIMENSIONAL_DATA_HPP

#include <vector>

#include "mdspan.hpp"
#include "multidimensional_block.hpp"
#include "processor_grid.hpp"
#include "profiler.hpp"

namespace reshuffle::internal {
    template<typename T, typename Extents>
    [[nodiscard]] auto get_1D_data(std::mdspan<const T, Extents> data) -> std::vector<T> {
        return std::vector<T>(data.data_handle(), data.data_handle() + data.size());
    }

    template<typename T, typename Extents>
    auto check_bounds(std::mdspan<const T, Extents> data,
                      const MultidimensionalBlock<Extents::rank()> &block) -> void {
        constexpr auto N = Extents::rank();

        for (int dim = 0; dim < N; ++dim) {
            const auto start = block[dim].get_interval().get_left_bound();
            const auto finish = start + block[dim].get_interval().get_length();
            if (start >= data.extent(dim) or finish > data.extent(dim)) {
                throw std::out_of_range("[check_bounds] Block is out of bounds");
            }
        }
    }

    template<typename T, typename Extents>
    [[nodiscard]] auto extract_data(std::mdspan<const T, Extents> data,
                                    const MultidimensionalBlock<Extents::rank()> &block)
            -> std::span<const T>
        requires(Extents::rank() == 1)
    {
        PROFILE_SCOPE_NAMED("extract_data");

        const auto start = block[0].get_interval().get_left_bound();
        const auto finish = start + block[0].get_interval().get_length();

        check_bounds(data, block);

        return {data.data_handle() + start, data.data_handle() + finish};
    }

    template<typename T, typename Extents>
    [[nodiscard]] auto extract_data(std::mdspan<const T, Extents> data,
                                    const MultidimensionalBlock<Extents::rank()> &block)
            -> std::vector<T>
        requires(Extents::rank() == 2)
    {
        PROFILE_SCOPE_NAMED("extract_data");
        check_bounds(data, block);

        auto values = std::vector<T>{};
        for (const auto i: block[0].get_interval()) {
            for (const auto j: block[1].get_interval()) { values.push_back(data[i, j]); }
        }

        return values;
    }

    template<typename T, typename Extents>
    [[nodiscard]] auto extract_data(std::mdspan<const T, Extents> data,
                                    const MultidimensionalBlock<Extents::rank()> &block)
            -> std::vector<T>
        requires(Extents::rank() == 3)
    {
        PROFILE_SCOPE_NAMED("extract_data");
        check_bounds(data, block);

        auto values = std::vector<T>{};
        for (const auto i: block[0].get_interval()) {
            for (const auto j: block[1].get_interval()) {
                for (const auto k: block[2].get_interval()) { values.push_back(data[i, j, k]); }
            }
        }

        return values;
    }

    template<typename T, typename Extents>
    auto copy_data(std::span<const T> origin, std::mdspan<T, Extents> destiny,
                   const MultidimensionalBlock<Extents::rank()> &local_block) -> void
        requires(Extents::rank() == 1)
    {
        PROFILE_SCOPE_NAMED("copy_data");
        const auto num_values_in_block = local_block[0].get_num_elements();
        const auto start_local = local_block[0].get_interval().get_left_bound();

        std::copy(origin.begin(), origin.begin() + num_values_in_block,
                  destiny.data_handle() + start_local);
    }

    template<typename T, typename Extents>
    auto copy_data(std::span<const T> origin, std::mdspan<T, Extents> destiny,
                   const MultidimensionalBlock<Extents::rank()> &local_block) -> void
        requires(Extents::rank() == 2)
    {
        PROFILE_SCOPE_NAMED("copy_data");
        int counter = 0;
        for (const auto i: local_block[0].get_interval()) {
            for (const auto j: local_block[1].get_interval()) { destiny[i, j] = origin[counter++]; }
        }
    }

    template<typename T, typename Extents>
    auto copy_data(std::span<const T> origin, std::mdspan<T, Extents> destiny,
                   const MultidimensionalBlock<Extents::rank()> &local_block) -> void
        requires(Extents::rank() == 3)
    {
        PROFILE_SCOPE_NAMED("copy_data");
        int counter = 0;
        for (const auto i: local_block[0].get_interval()) {
            for (const auto j: local_block[1].get_interval()) {
                for (const auto k: local_block[2].get_interval()) {
                    destiny[i, j, k] = origin[counter++];
                }
            }
        }
    }

    template<std::size_t N>
    [[nodiscard]] auto
    get_num_elements_per_processor(const std::vector<MultidimensionalBlock<N>> &blocks,
                                   const ProcessorGrid<N> &processor_grid)
            -> std::map<RankId, int> {
        PROFILE_SCOPE_NAMED("get_num_elements_per_processor");

        auto num_elements_per_process = std::map<RankId, int>{};

        for (const auto &block: blocks) {
            const auto owner_coordinates = get_owner_coordinates(block);
            const auto owner = processor_grid.get_processor_id(owner_coordinates);
            const auto num_elements = get_num_elements(block);
            num_elements_per_process[owner] += num_elements;
        }

        return num_elements_per_process;
    }

    template<std::size_t N>
    [[nodiscard]] auto group_by_processor(const std::vector<MultidimensionalBlock<N>> &blocks,
                                          const ProcessorGrid<N> &processor_grid)
            -> std::vector<Block> {
        PROFILE_SCOPE_NAMED("group_by_processor");

        if (blocks.empty()) { return {}; }

        const auto num_elements_per_process =
                get_num_elements_per_processor(blocks, processor_grid);
        auto blocks_grouped_by_owner = std::vector<Block>{};

        const auto [first_rank, first_length] = *num_elements_per_process.begin();
        blocks_grouped_by_owner.emplace_back(Block{{0, first_length}, first_rank});

        for (const auto &[rank, num_elements]: num_elements_per_process | std::views::drop(1)) {
            const auto last_inserted_block = blocks_grouped_by_owner.back();
            blocks_grouped_by_owner.emplace_back(
                    Block{{last_inserted_block.get_interval().get_right_bound(),
                           last_inserted_block.get_interval().get_right_bound() + num_elements},
                          rank});
        }

        return blocks_grouped_by_owner;
    }

}// namespace reshuffle::internal

#endif//MULTIDIMENSIONAL_DATA_HPP
