#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <shuffle.hpp>

using namespace reshuffle::internal;
using namespace reshuffle;

using testing::Eq;

enum class DataLocationSelector {
    ONLY_RANK_0,
    ONLY_RANK_1,
    ALL_RANKS,
};

[[nodiscard]] auto create_distribution(const DataLocationSelector &data_location,
                                       int num_global_values) -> BlockCyclic<1>;

TEST(GetSendAndReceiveBlocks, UsesAnOverlayToCheckWhatToSendAndWhatToReceive) {
    const auto blocks =
            std::vector{Block{{0, 2}, 0}, Block{{2, 3}, 0}, Block{{3, 4}, 1}, Block{{4, 5}, 0}};
    const auto owners_target_grid = std::vector{0, 1, 0, 1};
    const auto grid_overlay =
            GridOverlay{GridLayout{std::array{blocks}}, std::array{owners_target_grid}};

    const auto expected_send_0 = std::vector{Block{{0, 2}, 0}, Block{{2, 3}, 1}, Block{{3, 4}, 1}};
    const auto expected_receive_0 = std::vector{Block{{0, 2}, 0}, Block{{2, 3}, 1}};

    const auto expected_send_1 = std::vector{Block{{0, 1}, 0}};
    const auto expected_receive_1 = std::vector{Block{{0, 1}, 0}, Block{{1, 2}, 0}};

    const auto [send_blocks_0, receive_blocks_0] =
            get_send_and_receive_blocks(grid_overlay, {0}, {0});
    const auto [send_blocks_1, receive_blocks_1] =
            get_send_and_receive_blocks(grid_overlay, {1}, {1});

    EXPECT_THAT(send_blocks_0[0], Eq(expected_send_0));
    EXPECT_THAT(receive_blocks_0[0], Eq(expected_receive_0));
    EXPECT_THAT(send_blocks_1[0], Eq(expected_send_1));
    EXPECT_THAT(receive_blocks_1[0], Eq(expected_receive_1));
}

TEST(GetSendAndReceiveBlocks, WorkFromOneToMany) {
    constexpr auto num_values_per_rank = 6;
    constexpr auto num_ranks = 2;
    constexpr auto num_global_values = num_values_per_rank * num_ranks;

    const auto initial_distribution =
            create_distribution(DataLocationSelector::ONLY_RANK_0, num_global_values);
    const auto final_distribution =
            create_distribution(DataLocationSelector::ALL_RANKS, num_global_values);

    const auto &initial_grid = initial_distribution.get_grid_layout();
    const auto &final_grid = final_distribution.get_grid_layout();

    const auto &initial_processor_grid = initial_distribution.get_processor_grid();
    const auto &final_processor_grid = final_distribution.get_processor_grid();

    const auto overlay = initial_grid.get_overlay(final_grid, final_processor_grid);

    const auto [blocks_to_send_0, blocks_to_receive_0] =
            get_send_and_receive_blocks(overlay, {0}, {0});
    const auto [blocks_to_send_1, blocks_to_receive_1] =
            get_send_and_receive_blocks(overlay, {1}, {1});

    const auto blocks_to_send_0_expected = std::vector{Block{{0, 6}, 0}, Block{{6, 12}, 1}};
    const auto blocks_to_receive_0_expected = std::vector{Block{{0, 6}, 0}};

    constexpr auto blocks_to_send_1_expected = std::vector<Block>{};
    const auto blocks_to_receive_1_expected = std::vector{Block{{0, 6}, 0}};

    EXPECT_THAT(blocks_to_send_0, Eq(std::array{blocks_to_send_0_expected}));
    EXPECT_THAT(blocks_to_receive_0, Eq(std::array{blocks_to_receive_0_expected}));
    EXPECT_THAT(blocks_to_send_1, Eq(std::array{blocks_to_send_1_expected}));
    EXPECT_THAT(blocks_to_receive_1, Eq(std::array{blocks_to_receive_1_expected}));
}

auto create_distribution(const DataLocationSelector &data_location, const int num_global_values)
        -> BlockCyclic<1> {
    switch (data_location) {
        case DataLocationSelector::ONLY_RANK_0:
        case DataLocationSelector::ONLY_RANK_1:
            return make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{1}});
        case DataLocationSelector::ALL_RANKS:
            return make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{2}});
        default:
            throw std::runtime_error("Invalid DataLocationSelector");
    }
}