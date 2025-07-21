#ifndef COMMUNICATION_PACKAGE_HPP
#define COMMUNICATION_PACKAGE_HPP

#include <vector>

#include "block.hpp"
#include "mdspan.hpp"
#include "multidimensional_block.hpp"
#include "multidimensional_data.hpp"
#include "processor_grid.hpp"
#include "profiler.hpp"
#include "serialize.hpp"

namespace reshuffle::internal {
    template<typename T>
    struct SendCommunicationPackage {
        std::vector<T> buffer;
        std::map<rank_id, LeftClosedRange> data_assignments;

        [[nodiscard]] auto as_bytes() const -> SendCommunicationPackage<std::byte>;
    };

    template<typename T>
    struct ReceiveCommunicationPackage {
        std::vector<T> buffer;
        std::map<rank_id, LeftClosedRange> data_assignments;
    };

    template<typename T>
    auto SendCommunicationPackage<T>::as_bytes() const -> SendCommunicationPackage<std::byte> {
        if (buffer.empty()) {
            return {std::vector<std::byte>{}, std::map<rank_id, LeftClosedRange>{}};
        }

        auto new_data_assignments = std::map<rank_id, LeftClosedRange>{};
        auto serialized_buffer = std::vector<std::byte>{};

        // I need to order the map by interval to make sure that the data is packed in the same order
        // as in the original buffer. This is because I do not have an easy way to find beforehand
        // how much space each range will occupy once serialized
        auto data_assignments_ordered_by_interval =
                std::vector<std::pair<rank_id, LeftClosedRange>>{data_assignments.begin(),
                                                                 data_assignments.end()};
        std::ranges::sort(data_assignments_ordered_by_interval,
                          [](const auto &a, const auto &b) { return a.second < b.second; });

        auto old_size = 0;
        for (const auto &[rank, interval]: data_assignments_ordered_by_interval) {
            const auto bytes = serialize(std::span{buffer.data() + interval.get_left_bound(),
                                                   static_cast<size_t>(interval.get_length())});
            std::ranges::copy(bytes, std::back_inserter(serialized_buffer));

            const auto size = static_cast<int>(serialized_buffer.size());
            const auto data_interval = LeftClosedRange{old_size, size};

            new_data_assignments.emplace(rank, data_interval);

            old_size = size;
        }


        return {serialized_buffer, new_data_assignments};
    }


    auto get_starting_positions(const std::vector<Block> &blocks) -> std::map<rank_id, int>;

    template<typename T, typename Extents>
    auto get_send_package(std::mdspan<const T, Extents> local_data,
                          const std::vector<MultidimensionalBlock<Extents::rank()>> &send_blocks,
                          const ProcessorGrid<Extents::rank()> &final_processor_grid)
            -> SendCommunicationPackage<T> {
        PROFILE_SCOPE_NAMED("get_send_package");

        if (send_blocks.empty()) {
            return {std::vector<T>{}, std::map<rank_id, LeftClosedRange>{}};
        }

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

        std::map<rank_id, LeftClosedRange> data_assignments{};
        for (const auto &block: send_blocks_grouped_by_owner) {
            data_assignments.emplace(block.get_owner(), block.get_interval());
        }

        return {send_buffer, data_assignments};
    }

    template<typename T, std::size_t N>
    auto get_receive_package(const std::vector<MultidimensionalBlock<N>> &blocks_to_receive,
                             const ProcessorGrid<N> &initial_processor_grid)
            -> ReceiveCommunicationPackage<T> {
        PROFILE_SCOPE_NAMED("get_receive_package");

        if (blocks_to_receive.empty()) {
            return {std::vector<T>{}, std::map<rank_id, LeftClosedRange>{}};
        }

        const auto num_elements = get_num_elements(blocks_to_receive);
        auto receive_buffer = std::vector<T>(num_elements);

        const auto blocks_to_receive_grouped_by_owner =
                group_by_processor(blocks_to_receive, initial_processor_grid);

        std::map<rank_id, LeftClosedRange> data_assignments{};
        for (const auto &block: blocks_to_receive_grouped_by_owner) {
            data_assignments.emplace(block.get_owner(), block.get_interval());
        }

        return {receive_buffer, data_assignments};
    }

}// namespace reshuffle::internal

#endif//COMMUNICATION_PACKAGE_HPP
