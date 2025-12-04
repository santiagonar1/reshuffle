#include "coordinates.hpp"


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_block.hpp>

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
                                      MultidimensionalBlock{Block{{1, 3}, 0}, Block{{1, 2}, 1}}};

    EXPECT_THAT(make_contiguous(multi_blocks), Eq(expected));
}