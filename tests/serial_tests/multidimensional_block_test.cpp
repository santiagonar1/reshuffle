#include "coordinates.hpp"


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_block.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;

TEST(GetOwnerCoordinates, ReturnsTheCoordinatesOfProcessorThatOwnsMultiblock) {
    const auto x_block = Block{{0, 2}, 0};
    const auto y_block = Block{{1, 3}, 1};
    const auto multiblock = MultidimensionalBlock<2>{y_block, x_block};

    constexpr auto expected = Coordinates<2>{1, 0};
    EXPECT_THAT(get_owner_coordinates(multiblock), Eq(expected));
}

TEST(GetNumElements, ReturnsTheNumberOfElementsInTheBlock) {
    const auto x_block = Block{{0, 2}, 0};
    const auto y_block = Block{{1, 3}, 1};
    const auto multiblock = MultidimensionalBlock<2>{y_block, x_block};

    const auto expected = x_block.get_num_elements() * y_block.get_num_elements();
    EXPECT_THAT(get_num_elements(multiblock), Eq(expected));
}

TEST(GetNumElements, CanBeUsedInVectorOfBlocks) {
    const auto x_block = Block{{0, 2}, 0};
    const auto y_block = Block{{1, 3}, 1};
    const auto multiblock = MultidimensionalBlock<2>{y_block, x_block};

    const auto multi_blocks = std::vector{multiblock, multiblock};

    const auto expected = 2 * x_block.get_num_elements() * y_block.get_num_elements();
    EXPECT_THAT(get_num_elements(multi_blocks), Eq(expected));
}

TEST(ReplaceBlock, ChangesOneBlockOfMultidimensionalBlock) {
    const auto block = Block{{0, 2}, 0};
    const auto multiblock = MultidimensionalBlock<2>{block, block};

    const auto new_block = Block{{1, 3}, 1};
    const auto expected = MultidimensionalBlock{new_block, block};
    EXPECT_THAT(replace_block(multiblock, new_block, 0), Eq(expected));
}

TEST(ReplaceBlock, ThrowsIfDimOutsideOfBounds) {
    const auto block = Block{{0, 2}, 0};
    const auto multiblock = MultidimensionalBlock<2>{block, block};

    EXPECT_THROW(const auto _ = replace_block(multiblock, block, 2), std::invalid_argument);
}

TEST(MakeContiguous, MakesMultidimensionalBlocksContiguousInSpecifiedDimension) {
    constexpr int dim = 0;
    const auto first_multiblock = MultidimensionalBlock{Block{{0, 1}, 0}};
    const auto second_multiblock = MultidimensionalBlock{Block{{3, 5}, 1}};
    const auto multi_blocks = std::vector{first_multiblock, second_multiblock};

    const auto expected = std::vector{MultidimensionalBlock{{Block{{0, 1}, 0}}},
                                      MultidimensionalBlock{{Block{{1, 3}, 1}}}};
    EXPECT_THAT(make_contiguous(multi_blocks, dim), Eq(expected));
}

TEST(MakeContiguous, ThrowsIfDimOutsideOfBounds) {
    const auto multi_blocks = std::vector{MultidimensionalBlock{Block{{0, 1}, 0}}};
    EXPECT_THROW(const auto _ = make_contiguous(multi_blocks, 2), std::invalid_argument);
}

TEST(MakeContiguous, IfNoDimensionaParameterGivenMakesBlocksContiguousInAllDimensions) {
    const auto multi_blocks =
            std::vector{MultidimensionalBlock{Block{{0, 1}, 0}, Block{{1, 2}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 0}, Block{{1, 2}, 1}}};

    const auto expected = std::vector{MultidimensionalBlock{Block{{0, 1}, 0}, Block{{0, 1}, 1}},
                                      MultidimensionalBlock{Block{{1, 3}, 0}, Block{{0, 1}, 1}}};

    EXPECT_THAT(make_contiguous(multi_blocks), Eq(expected));
}

TEST(MakeContiguous, WorksWithVerticalBlocks) {
    const auto block_x = Block{{1, 2}, 0};
    const auto vertical_blocks = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}, block_x},
                                             MultidimensionalBlock{Block{{4, 5}, 1}, block_x}};

    const auto aligned_block_x = Block{{0, 1}, 0};
    const auto expected = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}, aligned_block_x},
                                      MultidimensionalBlock{Block{{2, 3}, 1}, aligned_block_x}};
    EXPECT_THAT(make_contiguous(vertical_blocks), Eq(expected));
}

TEST(MakeContiguous, WorksWithHorizontalBlocks) {
    const auto block_y = Block{{1, 2}, 0};
    const auto vertical_blocks = std::vector{MultidimensionalBlock{block_y, Block{{0, 2}, 0}},
                                             MultidimensionalBlock{block_y, Block{{4, 5}, 1}}};

    const auto aligned_block_y = Block{{0, 1}, 0};
    const auto expected = std::vector{MultidimensionalBlock{aligned_block_y, Block{{0, 2}, 0}},
                                      MultidimensionalBlock{aligned_block_y, Block{{2, 3}, 1}}};
    EXPECT_THAT(make_contiguous(vertical_blocks), Eq(expected));
}

TEST(GetDimensions, CalculatesTheDimensionsOfAMultidiemnsionalBlock) {
    const auto block = MultidimensionalBlock<2>{Block{{0, 1}, 0}, Block{{1, 3}, 1}};

    constexpr auto expected = Dimensions{1, 2};
    EXPECT_THAT(get_dimensions(block), Eq(expected));
}

TEST(GetDimensions, CalculatesTheDimensionsOfVectorOfMultidimensionalBlockByAddingEachDimension) {
    // The first block is 1x2, and the second one 2x2
    const auto blocks = std::vector{MultidimensionalBlock<2>{Block{{0, 1}, 0}, Block{{1, 3}, 1}},
                                    MultidimensionalBlock<2>{Block{{2, 4}, 1}, Block{{3, 5}, 0}}};

    constexpr auto expected = Dimensions{3, 4};
    EXPECT_THAT(get_dimensions(blocks), Eq(expected));
}