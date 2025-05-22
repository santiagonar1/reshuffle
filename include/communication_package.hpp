#ifndef COMMUNICATION_PACKAGE_HPP
#define COMMUNICATION_PACKAGE_HPP

#include <vector>

#include "block.hpp"
#include "mdspan.hpp"
#include "multidimensional_block.hpp"
#include "multidimensional_data.hpp"
#include "processor_grid.hpp"
#include "profiler.hpp"

namespace reshuffle::internal {
    template<typename T>
    struct CommunicationPackage {
        std::vector<T> buffer;
        std::vector<Block> data_assignments;
    };

    auto get_starting_positions(const std::vector<Block> &blocks) -> std::map<rank_id, int>;

    template<typename T, typename Extents>
    auto get_send_package(std::mdspan<const T, Extents> local_data,
                          const std::vector<MultidimensionalBlock<Extents::rank()>> &send_blocks,
                          const ProcessorGrid<Extents::rank()> &final_processor_grid)
            -> CommunicationPackage<T> {
        PROFILE_SCOPE_NAMED("get_send_package");

        if (send_blocks.empty()) { return {std::vector<T>{}, std::vector<Block>{}}; }

        auto send_buffer = std::vector<T>(local_data.size());

        const auto send_blocks_grouped_by_owner =
                group_by_processor(send_blocks, final_processor_grid);
        const auto starting_positions = get_starting_positions(send_blocks_grouped_by_owner);

        auto num_elements_packed_per_process = std::map<rank_id, int>{};

        for (const auto &block: send_blocks) {
            const auto destiny =
                    final_processor_grid.get_processor_id(get_owner_coordinates(block));

            const auto num_elements_packed = num_elements_packed_per_process[destiny];
            const auto starting_position = starting_positions.at(destiny) + num_elements_packed;

            const auto block_data = extract_data(local_data, block);
            std::ranges::copy(block_data, send_buffer.begin() + starting_position);

            num_elements_packed_per_process[destiny] += block_data.size();
        }

        return {send_buffer, send_blocks_grouped_by_owner};
    }

    template<typename T, std::size_t N>
    auto get_receive_package(const std::vector<MultidimensionalBlock<N>> &blocks_to_receive,
                             const ProcessorGrid<N> &initial_processor_grid)
            -> CommunicationPackage<T> {
        PROFILE_SCOPE_NAMED("get_receive_package");

        if (blocks_to_receive.empty()) { return {std::vector<T>{}, std::vector<Block>{}}; }

        const auto num_elements = get_num_elements(blocks_to_receive);
        auto receive_buffer = std::vector<T>(num_elements);

        const auto blocks_to_receive_grouped_by_owner =
                group_by_processor(blocks_to_receive, initial_processor_grid);

        return {receive_buffer, blocks_to_receive_grouped_by_owner};
    }

}// namespace reshuffle::internal

#endif//COMMUNICATION_PACKAGE_HPP
