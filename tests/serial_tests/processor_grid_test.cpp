#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <processor_grid.hpp>

using namespace reshuffle;

using testing::Eq;

TEST(ProcessorGrid, ReturnsNumberOfProcessorsInGrid) {
    constexpr auto num_processors = 4;
    const auto processor_grid = ProcessorGrid{num_processors};

    EXPECT_THAT(processor_grid.get_num_processors(), Eq(num_processors));
}

TEST(ProcessorGrid, EnumeratesProcessorsRowWise) {
    const auto processor_grid = ProcessorGrid{2, 2};

    EXPECT_THAT(processor_grid.get_processor_id({0, 0}), Eq(0));
    EXPECT_THAT(processor_grid.get_processor_id({0, 1}), Eq(1));
    EXPECT_THAT(processor_grid.get_processor_id({1, 0}), Eq(2));
    EXPECT_THAT(processor_grid.get_processor_id({1, 1}), Eq(3));
}

TEST(ProcessorGrid, GetsProcessorCoordinates) {
    const auto processor_grid = ProcessorGrid{2, 2};

    EXPECT_THAT(processor_grid.get_processor_coordinates(0), Eq(Coordinates<2>{0, 0}));
    EXPECT_THAT(processor_grid.get_processor_coordinates(1), Eq(Coordinates<2>{0, 1}));
    EXPECT_THAT(processor_grid.get_processor_coordinates(2), Eq(Coordinates<2>{1, 0}));
    EXPECT_THAT(processor_grid.get_processor_coordinates(3), Eq(Coordinates<2>{1, 1}));
}
