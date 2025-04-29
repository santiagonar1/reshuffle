#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <shuffle.hpp>

using namespace reshuffle::dev::internal;
using namespace reshuffle::dev;

using testing::Eq;

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

    const auto [send_blocks_0, receive_blocks_0] = get_send_and_receive_blocks(grid_overlay, 0, 0);
    const auto [send_blocks_1, receive_blocks_1] = get_send_and_receive_blocks(grid_overlay, 1, 1);

    EXPECT_THAT(send_blocks_0, Eq(expected_send_0));
    EXPECT_THAT(receive_blocks_0, Eq(expected_receive_0));
    EXPECT_THAT(send_blocks_1, Eq(expected_send_1));
    EXPECT_THAT(receive_blocks_1, Eq(expected_receive_1));
}