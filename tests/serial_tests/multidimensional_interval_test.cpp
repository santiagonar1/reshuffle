#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_interval.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::UnorderedElementsAreArray;

TEST(MultidimensionalInterval, IsComposedOfIntervals) {
    const auto interval = MultidimensionalInterval{Interval{0, 2}, Interval{0, 2}};
}

TEST(ToUnidimensionalIntervals, DecomposeMultidimensionalInterval) {
    const auto interval = MultidimensionalInterval{Interval{0, 2}, Interval{2, 5}};

    const auto expected = std::vector{Interval{0, 2}, Interval{2, 5}};
    EXPECT_THAT(to_unidimensional_intervals(interval), UnorderedElementsAreArray(expected));
}

TEST(ToUnidimensionalIntervals, DecomposeVectorOfMultidimensionalIntervals) {
    const auto intervals = std::vector{MultidimensionalInterval{Interval{0, 2}, Interval{2, 5}},
                                       MultidimensionalInterval{Interval{0, 2}, Interval{1, 3}}};

    const auto expected_y = std::vector{Interval{0, 2}};
    const auto expected_x = std::vector{Interval{2, 5}, Interval{1, 3}};

    const auto result = to_unidimensional_intervals(intervals);
    EXPECT_THAT(result[0], UnorderedElementsAreArray(expected_y));
    EXPECT_THAT(result[1], UnorderedElementsAreArray(expected_x));
}