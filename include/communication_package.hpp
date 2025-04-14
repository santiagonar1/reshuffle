#ifndef COMMUNICATION_PACKAGE_HPP
#define COMMUNICATION_PACKAGE_HPP

#include <vector>

#include "block.hpp"

namespace reshuffle::dev::internal {
    template<typename T>
    struct CommunicationPackage {
        std::vector<T> buffer;
        std::vector<Block> data_assignments;
    };

    auto get_starting_positions(const std::vector<Block> &blocks) -> std::map<rank_id, int>;

    template<typename T>
    auto get_send_package(std::span<const T> local_data, const std::vector<Block> &send_blocks)
            -> CommunicationPackage<T> {

        if (send_blocks.empty()) { return {std::vector<T>{}, std::vector<Block>{}}; }

        auto send_buffer = std::vector<T>(local_data.size());

        const auto send_blocks_grouped_by_owner = group_by_processor(send_blocks);
        const auto starting_positions = get_starting_positions(send_blocks_grouped_by_owner);

        auto num_elements_packed_per_process = std::map<rank_id, int>{};

        for (const auto &block: send_blocks) {
            const auto destiny = block.get_owner();

            const auto num_elements_packed = num_elements_packed_per_process[destiny];
            const auto starting_position = starting_positions.at(destiny) + num_elements_packed;

            const auto block_data = extract_data(local_data, block);
            std::ranges::copy(block_data, send_buffer.begin() + starting_position);

            num_elements_packed_per_process[destiny] += block_data.size();
        }

        return {send_buffer, send_blocks_grouped_by_owner};
    }

    template<typename T>
    auto get_receive_package(const std::vector<Block> &blocks_to_receive)
            -> CommunicationPackage<T> {

        if (blocks_to_receive.empty()) { return {std::vector<T>{}, std::vector<Block>{}}; }

        const auto num_elements = blocks_to_receive.back().get_interval().get_right_bound();
        auto receive_buffer = std::vector<T>(num_elements);

        const auto blocks_to_receive_grouped_by_owner = group_by_processor(blocks_to_receive);

        return {receive_buffer, blocks_to_receive_grouped_by_owner};
    }

}// namespace reshuffle::dev::internal

#endif//COMMUNICATION_PACKAGE_HPP
