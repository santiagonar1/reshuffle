#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <data_exchanger.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;
using testing::UnorderedElementsAreArray;

TEST(GetSendAndReceiveBlocks, ReturnsMultidimensionalBlocksToSendAndReceive) {
    // 0     2     4
    // |--0--|--1--|
    const auto origin_blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    // 0           4
    // |-----0-----|
    const auto target_blocks = std::vector{Block{{0, 4}, 0}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    // 0     2     4
    // |-----|-----|
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // Rank 0
    const auto expected_send_0 = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}}};
    const auto expected_receive_0 = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}},
                                                MultidimensionalBlock{Block{{2, 4}, 1}}};

    const auto [send_0, receive_0] =
            get_send_and_receive_blocks_dev(overlay, {0}, {0}, IntervalType::GLOBAL);
    EXPECT_THAT(send_0, Eq(expected_send_0));
    EXPECT_THAT(receive_0, Eq(expected_receive_0));

    // Rank 1
    const auto expected_send_1 = std::vector{MultidimensionalBlock{Block{{2, 4}, 0}}};
    constexpr auto expected_receive_1 = std::vector<MultidimensionalBlock<1>>{};

    const auto [send_1, receive_1] =
            get_send_and_receive_blocks_dev(overlay, {1}, {INVALID_RANK_ID}, IntervalType::GLOBAL);
    EXPECT_THAT(send_1, Eq(expected_send_1));
    EXPECT_THAT(receive_1, Eq(expected_receive_1));
}

TEST(GetSendAndReceiveBlocks, CanReturnLocalIntervals) {
    // 0     2     4
    // |--0--|--1--|
    const auto origin_blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    // 0           4
    // |-----0-----|
    const auto target_blocks = std::vector{Block{{0, 4}, 0}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    // 0     2     4
    // |-----|-----|
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // Rank 1
    const auto expected_send_1 = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}}};
    constexpr auto expected_receive_1 = std::vector<MultidimensionalBlock<1>>{};

    const auto [send_1, receive_1] =
            get_send_and_receive_blocks_dev(overlay, {1}, {INVALID_RANK_ID}, IntervalType::LOCAL);
    EXPECT_THAT(send_1, Eq(expected_send_1));
    EXPECT_THAT(receive_1, Eq(expected_receive_1));
}

TEST(GetSendAndReceiveBlocks, WorksFromOneToMany) {
    // 0           4
    // |-----1-----|
    const auto origin_blocks = std::vector{Block{{0, 4}, 1}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    // 0     2     4
    // |--0--|--1--|
    const auto target_blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    // 0     2     4
    // |-----|-----|
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // Rank 0
    constexpr auto expected_send_0 = std::vector<MultidimensionalBlock<1>>{};
    const auto expected_receive_0 = std::vector{MultidimensionalBlock{Block{{0, 2}, 1}}};

    const auto [send_0, receive_0] =
            get_send_and_receive_blocks_dev(overlay, {INVALID_RANK_ID}, {0}, IntervalType::GLOBAL);
    EXPECT_THAT(send_0, Eq(expected_send_0));
    EXPECT_THAT(receive_0, Eq(expected_receive_0));

    // Rank 1
    const auto expected_send_1 = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}},
                                             MultidimensionalBlock{Block{{2, 4}, 1}}};
    const auto expected_receive_1 = std::vector{MultidimensionalBlock{Block{{2, 4}, 1}}};

    const auto [send_1, receive_1] =
            get_send_and_receive_blocks_dev(overlay, {1}, {1}, IntervalType::GLOBAL);
    EXPECT_THAT(send_1, Eq(expected_send_1));
    EXPECT_THAT(receive_1, Eq(expected_receive_1));
}

TEST(GetSendAndReceiveBlocks, WorksIn2D) {

    //                 4        6
    //   +-------------+---------+
    //   |             |         |
    //   |             |         |
    //   |    (0,0)    |  (0,1)  |
    //   |             |         |
    //   |             |         |
    // 5 +-------------+---------+
    //   |    (1,0)    |  (1,1)  |
    // 6 +-------------+---------+
    const auto origin_blocks_x = std::vector{Block{{0, 4}, 0}, Block{{4, 6}, 1}};
    const auto origin_blocks_y = std::vector{Block{{0, 5}, 0}, Block{{5, 6}, 1}};
    const auto origin_grid = GridLayout(std::array{origin_blocks_y, origin_blocks_x});


    //           2              6
    //   +-------+---------------+
    //   |       |               |
    //   | (0,0) |     (0,1)     |
    //   |       |               |
    // 3 +-------+---------------+
    //   |       |               |
    //   | (1,0) |     (1,1)     |
    //   |       |               |
    // 6 +-------+---------------+
    const auto target_blocks_x = std::vector{Block{{0, 2}, 0}, Block{{2, 6}, 1}};
    const auto target_blocks_y = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}};
    const auto target_grid = GridLayout(std::array{target_blocks_y, target_blocks_x});

    //           2      4       6
    //   +-------+------+--------+
    //   |       |      |        |
    //   |       |      |        |
    //   |       |      |        |
    // 3 +-------+------+--------+
    //   |       |      |        |
    // 5 +-------+------+--------+
    //   |       |      |        |
    // 6 +-------+------+--------+
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // Rank 0 -> (0, 0)
    const auto expected_send_0 =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{2, 4}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{2, 4}, 1}}};
    const auto expected_receive_0 =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{0, 2}, 0}}};

    const auto [send_0, receive_0] =
            get_send_and_receive_blocks_dev(overlay, {0, 0}, {0, 0}, IntervalType::GLOBAL);
    EXPECT_THAT(send_0, UnorderedElementsAreArray(expected_send_0));
    EXPECT_THAT(receive_0, UnorderedElementsAreArray(expected_receive_0));

    // Rank 1 -> (0, 1)
    const auto expected_send_1 =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{4, 6}, 1}}};
    const auto expected_receive_1 =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{4, 6}, 1}}};

    const auto [send_1, receive_1] =
            get_send_and_receive_blocks_dev(overlay, {0, 1}, {0, 1}, IntervalType::GLOBAL);
    EXPECT_THAT(send_1, UnorderedElementsAreArray(expected_send_1));
    EXPECT_THAT(receive_1, UnorderedElementsAreArray(expected_receive_1));

    // Rank 2 -> (1, 0)
    const auto expected_send_2 =
            std::vector{MultidimensionalBlock{Block{{5, 6}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{2, 4}, 1}}};
    const auto expected_receive_2 =
            std::vector{MultidimensionalBlock{Block{{3, 5}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{0, 2}, 0}}};

    const auto [send_2, receive_2] =
            get_send_and_receive_blocks_dev(overlay, {1, 0}, {1, 0}, IntervalType::GLOBAL);
    EXPECT_THAT(send_2, UnorderedElementsAreArray(expected_send_2));
    EXPECT_THAT(receive_2, UnorderedElementsAreArray(expected_receive_2));

    // Rank 3 -> (1, 1)
    const auto expected_send_3 =
            std::vector{MultidimensionalBlock{Block{{5, 6}, 1}, Block{{4, 6}, 1}}};
    const auto expected_receive_3 =
            std::vector{MultidimensionalBlock{Block{{3, 5}, 0}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{3, 5}, 0}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{4, 6}, 1}}};

    const auto [send_3, receive_3] =
            get_send_and_receive_blocks_dev(overlay, {1, 1}, {1, 1}, IntervalType::GLOBAL);
    EXPECT_THAT(send_3, UnorderedElementsAreArray(expected_send_3));
    EXPECT_THAT(receive_3, UnorderedElementsAreArray(expected_receive_3));
}