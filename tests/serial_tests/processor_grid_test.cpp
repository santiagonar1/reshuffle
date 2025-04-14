#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <processor_grid.hpp>

using namespace reshuffle::dev;

using testing::Eq;

TEST(ProcessorGrid, ReturnsNumberOfProcessorsInGrid) {
    constexpr auto num_processors = 4;
    const auto processor_grid = ProcessorGrid(num_processors);

    EXPECT_THAT(processor_grid.get_num_processors(), Eq(num_processors));
}