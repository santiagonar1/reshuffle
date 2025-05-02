#include "coordinates.hpp"


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_block.hpp>

using namespace reshuffle::dev;

using testing::Eq;

TEST(GetOwnerCoordinates, ReturnsTheCoordinatesOfProcessorThatOwnsMultiblock) {
    const auto x_block = Block{{0, 2}, 0};
    const auto y_block = Block{{1, 3}, 1};
    const auto multiblock = MultidimensionalBlock{y_block, x_block};

    const auto expected = reshuffle::internal::Coordinates{1, 0};
    EXPECT_THAT(get_owner_coordinates(multiblock), Eq(expected));
}