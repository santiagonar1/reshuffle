#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_overlay.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;

TEST(GetCoordinatesOrigin, ReturnCoordinatesOriginOfMultidimensionalOverlay) {
    const auto overlay =
            MultidimensionalOverlay<2>{BlockOverlay{{0, 1}, 0, 1}, BlockOverlay{{0, 1}, 1, 0}};
    constexpr auto expected = Coordinates<2>{0, 1};
    EXPECT_THAT(get_coordinates_origin(overlay), Eq(expected));
}

TEST(GetCoordinatesTarget, ReturnCoordinatesTargetOfMultidimensionalOverlay) {
    const auto overlay =
            MultidimensionalOverlay<2>{BlockOverlay{{0, 1}, 0, 1}, BlockOverlay{{0, 1}, 1, 0}};
    constexpr auto expected = Coordinates<2>{1, 0};
    EXPECT_THAT(get_coordinates_target(overlay), Eq(expected));
}

TEST(GetMultidimensionalBlockOrigin, ReturnsAMultidimensionalBlockWithOriginIds) {
    const auto overlay =
            MultidimensionalOverlay<2>{BlockOverlay{{0, 1}, 0, 1}, BlockOverlay{{0, 1}, 1, 0}};

    const auto expected = MultidimensionalBlock<2>{Block{{0, 1}, 0}, Block{{0, 1}, 1}};
    EXPECT_THAT(get_multidimensional_block_origin(overlay), Eq(expected));
}

TEST(GetMultidimensionalBlockTarget, ReturnsAMultidimensionalBlockWithTargetIds) {
    const auto overlay =
            MultidimensionalOverlay<2>{BlockOverlay{{0, 1}, 0, 1}, BlockOverlay{{0, 1}, 1, 0}};

    const auto expected = MultidimensionalBlock<2>{Block{{0, 1}, 1}, Block{{0, 1}, 0}};
    EXPECT_THAT(get_multidimensional_block_target(overlay), Eq(expected));
}