#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_overlay.hpp>

using namespace reshuffle::internal;

using testing::Eq;

TEST(GetBlocksOverlay, CalculatesBlockOverlaysOfTwoBlocks) {
    const auto origin = Block{{0, 2}, 0};
    const auto target = Block{{0, 1}, 1};

    const auto expected = BlockOverlay{.interval = {0, 1}, .id_origin = 0, .id_target = 1};
    EXPECT_THAT(get_blocks_overlay(origin, target), Eq(expected));
}

TEST(GetBlocksOverlay, ReturnsNulloptIfNoOverlay) {
    const auto origin = Block{{0, 2}, 0};
    const auto non_overlapping_target = Block{{2, 3}, 0};

    EXPECT_FALSE(get_blocks_overlay(origin, non_overlapping_target).has_value());
}

TEST(GetBlocksOverlay, CalculatesOverlaysOfVectorsOfBlocks) {
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto target_blocks = std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};

    const auto expected = std::vector{BlockOverlay{{0, 1}, 0, 0}, BlockOverlay{{1, 2}, 0, 1},
                                      BlockOverlay{{2, 4}, 1, 1}};
    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}

TEST(GetBlocksOverlay, ThrowsIfInitialBlocksDoNotStartAtSameIndex) {
    const auto origin_blocks = std::vector{Block{{0, 3}, 0}};
    const auto unaligned_target_blocks = std::vector{Block{{1, 2}, 0}};

    EXPECT_THROW(const auto _ = get_blocks_overlay(origin_blocks, unaligned_target_blocks),
                 std::invalid_argument);
}

TEST(GetBlocksOverlay, ThrowsIfFinalBlocksDoNotEndAtSameIndex) {
    const auto origin_blocks = std::vector{Block{{0, 3}, 0}};
    const auto unaligned_target_blocks = std::vector{Block{{0, 2}, 0}};

    EXPECT_THROW(const auto _ = get_blocks_overlay(origin_blocks, unaligned_target_blocks),
                 std::invalid_argument);
}

TEST(GetBlocksOverlay, WorksWhenTargetGridHasLargerIntervalAtSomePoint) {
    const auto origin_blocks = std::vector{Block{{0, 3}, 0}, Block{{3, 4}, 1}};
    const auto target_blocks = std::vector{Block{{0, 4}, 1}};

    const auto expected = std::vector{BlockOverlay{{0, 3}, 0, 1}, BlockOverlay{{3, 4}, 1, 1}};
    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}

TEST(GetBlocksOverlay, WorksWhenSeveralTargetBlocksContainedInOneOriginBlock) {
    const auto origin_blocks = std::vector{Block{{0, 4}, 0}};
    const auto target_blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}};

    const auto expected = std::vector{BlockOverlay{{0, 2}, 0, 0}, BlockOverlay{{2, 4}, 0, 1}};
    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}

TEST(GetBlocksOverlay, WorksFromManyToOne) {
    const auto origin_blocks = std::vector{{Block{{0, 4}, 0}, Block{{4, 8}, 1}, Block{{8, 12}, 1}}};
    const auto target_blocks = std::vector{{Block{{0, 12}, 0}}};

    const auto expected = std::vector{BlockOverlay{{0, 4}, 0, 0}, BlockOverlay{{4, 8}, 1, 0},
                                      BlockOverlay{{8, 12}, 1, 0}};
    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}

TEST(GetBlocksOverlay, WorksFromOneToMany) {
    const auto origin_blocks = std::vector{Block{{0, 12}, 0}};
    const auto target_blocks = std::vector{{Block{{0, 4}, 0}, Block{{4, 8}, 1}, Block{{8, 12}, 1}}};

    const auto expected = std::vector{BlockOverlay{{0, 4}, 0, 0}, BlockOverlay{{4, 8}, 0, 1},
                                      BlockOverlay{{8, 12}, 0, 1}};
    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}

TEST(GetBlocksOverlay, WorksWithMultidimensionalArrays) {
    const auto origin_blocks_dim_0 = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_blocks_dim_1 = std::vector{{Block{{0, 1}, 1}}};

    const auto target_blocks_dim_0 =
            std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto target_blocks_dim_1 = std::vector{{Block{{0, 1}, 0}}};

    const auto origin_blocks = std::array{origin_blocks_dim_0, origin_blocks_dim_1};
    const auto target_blocks = std::array{target_blocks_dim_0, target_blocks_dim_1};

    const auto expected_dim_0 = std::vector{BlockOverlay{{0, 1}, 0, 0}, BlockOverlay{{1, 2}, 0, 1},
                                            BlockOverlay{{2, 4}, 1, 1}};
    const auto expected_dim_1 = std::vector{BlockOverlay{{0, 1}, 1, 0}};

    const auto expected = std::array{expected_dim_0, expected_dim_1};

    EXPECT_THAT(get_blocks_overlay(origin_blocks, target_blocks), Eq(expected));
}