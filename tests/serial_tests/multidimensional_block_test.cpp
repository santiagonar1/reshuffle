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