#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <coordinates.hpp>

using namespace reshuffle::internal;
using namespace reshuffle;

using testing::Eq;

TEST(Coordinates, CanBeCreatedFromArray) {
    constexpr auto coordinates = Coordinates<2>{1, 2};
    EXPECT_THAT(coordinates[0], Eq(1));
    EXPECT_THAT(coordinates[1], Eq(2));
}

TEST(MapIndices, MapsAMultiDimensionCoordinateTo1D) {
    constexpr auto coordinates = Coordinates<2>{0, 1};
    constexpr auto dimensions = Dimensions<2>{2, 4};

    EXPECT_THAT(map_indices(coordinates, dimensions), Eq(1));
}

TEST(MapIndices, UsesRowMajorFormat) {
    constexpr auto dimensions = Dimensions<2>{2, 4};

    EXPECT_THAT(map_indices(Coordinates<2>{0, 3}, dimensions), Eq(3));
    EXPECT_THAT(map_indices(Coordinates<2>{1, 0}, dimensions), Eq(4));
}

TEST(MapIndices, EachCoordianteIndexIsBoundedByTheRespectiveDimension) {
    constexpr auto dimensions = Dimensions<2>{2, 4};
    constexpr auto coordinates_too_many_rows = Coordinates<2>{2, 0};
    constexpr auto coordinates_too_many_columns = Coordinates<2>{0, 4};

    EXPECT_FALSE(map_indices(coordinates_too_many_rows, dimensions).has_value());
    EXPECT_FALSE(map_indices(coordinates_too_many_columns, dimensions).has_value());
}

TEST(MapIndex, MapsA1DIndexIntoAMultiDimensionalCoordinate) {
    constexpr auto dimensions = Dimensions<2>{2, 4};

    EXPECT_THAT(map_index(0, dimensions), Eq(Coordinates<2>{0, 0}));
    EXPECT_THAT(map_index(3, dimensions), Eq(Coordinates<2>{0, 3}));
    EXPECT_THAT(map_index(4, dimensions), Eq(Coordinates<2>{1, 0}));
}

TEST(MapIndex, UsesRowMajorFormat) {
    constexpr auto dimensions = Dimensions<2>{2, 4};

    EXPECT_THAT(map_index(0, dimensions), Eq(Coordinates<2>{0, 0}));
    EXPECT_THAT(map_index(1, dimensions), Eq(Coordinates<2>{0, 1}));
    EXPECT_THAT(map_index(dimensions[1], dimensions), Eq(Coordinates<2>{1, 0}));
}

TEST(MapIndex, ReturnsNoValueIf1DIndexOutsideOfNumValuesHeldByDimensions) {
    constexpr auto dimensions = Dimensions<2>{2, 4};
    const auto num_values = calc_total_num_values(dimensions);

    EXPECT_FALSE(map_index(num_values, dimensions).has_value());
}