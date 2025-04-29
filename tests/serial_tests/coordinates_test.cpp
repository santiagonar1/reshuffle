#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <coordinates.hpp>

using namespace reshuffle::internal;
using namespace reshuffle;

using testing::Eq;

TEST(Coordinates, CanBeCreatedFromArray) {
    const auto coordinates = Coordinates{1, 2};
    EXPECT_THAT(coordinates[0], Eq(1));
    EXPECT_THAT(coordinates[1], Eq(2));
}

TEST(MapIndices, MapsAMultiDimensionCoordinateTo1D) {
    const auto coordinates = Coordinates{0, 1};
    const auto dimensions = Dimensions{2, 4};

    EXPECT_THAT(map_indices(coordinates, dimensions), Eq(1));
}

TEST(MapIndices, UsesRowMajorFormat) {
    const auto dimensions = Dimensions{4, 2};

    EXPECT_THAT(map_indices(Coordinates{0, 3}, dimensions), Eq(3));
    EXPECT_THAT(map_indices(Coordinates{1, 0}, dimensions), Eq(4));
}

TEST(MapIndices, EachCoordianteIndexIsBoundedByTheRespectiveDimension) {
    const auto dimensions = Dimensions{4, 2};
    const auto coordinates_too_many_rows = Coordinates{2, 0};
    const auto coordinates_too_many_columns = Coordinates{0, 4};

    EXPECT_THROW(auto _ = map_indices(coordinates_too_many_rows, dimensions),
                 std::invalid_argument);
    EXPECT_THROW(auto _ = map_indices(coordinates_too_many_columns, dimensions),
                 std::invalid_argument);
}

TEST(MapIndex, MapsA1DIndexIntoAMultiDimensionalCoordinate) {
    const auto dimensions = Dimensions{4, 2};

    EXPECT_THAT(map_index(0, dimensions), Eq(Coordinates{0, 0}));
    EXPECT_THAT(map_index(3, dimensions), Eq(Coordinates{0, 3}));
    EXPECT_THAT(map_index(4, dimensions), Eq(Coordinates{1, 0}));
}

TEST(MapIndex, UsesRowMajorFormat) {
    const auto dimensions = Dimensions{4, 2};

    EXPECT_THAT(map_index(0, dimensions), Eq(Coordinates{0, 0}));
    EXPECT_THAT(map_index(1, dimensions), Eq(Coordinates{0, 1}));
    EXPECT_THAT(map_index(dimensions[0], dimensions), Eq(Coordinates{1, 0}));
}

TEST(MapIndex, ThrowsIf1DIndexOutsideOfNumValuesHeldByDimensions) {
    const auto dimensions = Dimensions{2, 4};
    const auto num_values = calc_total_num_values(dimensions);

    EXPECT_THROW(auto _ = map_index(num_values, dimensions), std::invalid_argument);
}