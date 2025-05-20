#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block.hpp>

using namespace reshuffle::internal;

using testing::Eq;

TEST(Block, CanCalculateAnOverlay) {
    const auto block = Block{{0, 2}, 0};
    const auto other_block = Block{{1, 2}, 0};

    const auto expected_overlay = Block{{1, 2}, 0};

    EXPECT_THAT(block.get_overlay(other_block), Eq(expected_overlay));
}

TEST(Block, OverlayHasNoValueIfNoOverlay) {
    const auto block = Block{{0, 2}, 0};
    const auto other_block = Block{{2, 3}, 0};

    EXPECT_FALSE(block.get_overlay(other_block).has_value());
}

TEST(Block, ReturnsTheNumberOfElementsInTheBlock) {
    const auto block = Block{{0, 2}, 0};

    const auto expected = block.get_interval().get_length();
    EXPECT_THAT(block.get_num_elements(), Eq(expected));
}

TEST(Join, CreatesFromADisjointVectorOfBlocksAUnifiedList) {
    const auto blocks = std::vector{Block{{0, 2}, 0}, Block{{3, 4}, 1}, Block{{7, 8}, 0}};
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 3}, 1}, Block{{3, 4}, 0}};

    EXPECT_THAT(join(blocks), Eq(expected));
}

TEST(ExtractData, TakesABufferAndReturnsAViewOfTheDataContainedByTheBlock) {
    const auto data = std::vector{0, 1, 2, 3, 4, 5};
    const auto block = Block{{0, 2}, 0};

    const auto expected = std::vector{0, 1};
    const auto result_view = extract_data(std::span{data}, block);
    const auto result = std::vector(result_view.begin(), result_view.end());

    EXPECT_THAT(result, Eq(expected));
}

TEST(ExtractData, ThrowsIfBlockIsOutOfBounds) {
    const auto data = std::vector{0, 1, 2, 3, 4, 5};
    const auto block_right_bound_out_of_bounds = Block{{0, static_cast<int>(data.size()) + 1}, 0};
    const auto block_left_bound_out_of_bounds =
            Block{{static_cast<int>(data.size()), static_cast<int>(data.size()) + 20}, 0};

    EXPECT_THROW(extract_data(std::span{data}, block_right_bound_out_of_bounds), std::out_of_range);
    EXPECT_THROW(extract_data(std::span{data}, block_left_bound_out_of_bounds), std::out_of_range);
}

TEST(GetNumElementsPerOwner, ReturnsAMapWithTheNumberOfElementsAssignToEachProcessor) {
    const auto blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}, Block{{4, 6}, 0}};
    const auto expected = std::map{std::pair{0, 4}, std::pair{1, 2}};

    EXPECT_THAT(get_num_elements_per_processor(blocks), Eq(expected));
}

TEST(GroupByOwner, TakesAVectorOfBlocksAndGroupsIntervalsByOwner) {
    const auto blocks = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}, Block{{4, 6}, 0}};
    const auto expected = std::vector{Block{{0, 4}, 0}, Block{{4, 6}, 1}};

    EXPECT_THAT(group_by_processor(blocks), Eq(expected));
}

TEST(GroupByOwner, ReturnsIntervalsOrderByOwner) {
    const auto blocks = std::vector{Block{{0, 2}, 1}, Block{{2, 4}, 0}};
    const auto expected = std::vector{Block{{0, 2}, 0}, Block{{2, 4}, 1}};

    EXPECT_THAT(group_by_processor(blocks), Eq(expected));
}