#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <left_closed_range.hpp>

using namespace reshuffle;

using testing::Eq;
using testing::Ne;


TEST(LeftClosedRange, ContainsLeftBound) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = 7;

    const auto range = LeftClosedRange(left_bound, right_bound);

    EXPECT_TRUE(range.contains(left_bound));
}

TEST(LeftClosedRange, DoesNotContainRightBound) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = 7;

    const auto range = LeftClosedRange(left_bound, right_bound);

    EXPECT_FALSE(range.contains(right_bound));
}

TEST(LeftClosedRange, FromAToBContainsBMinusAElements) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = 7;
    constexpr auto length = right_bound - left_bound;

    const auto range = LeftClosedRange(left_bound, right_bound);

    EXPECT_THAT(range.get_length(), Eq(length));
}

TEST(LeftClosedRange, WithSameLeftAndRightBoundsHaveZeroLength) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = left_bound;

    const auto range = LeftClosedRange(left_bound, right_bound);
    EXPECT_THAT(range.get_length(), Eq(0));
}

TEST(LeftClosedRange, IsEqualToOtherLeftClosedRangeIfTheyHaveSameBounds) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = 7;

    const auto range = LeftClosedRange(left_bound, right_bound);
    const auto same_range = LeftClosedRange(left_bound, right_bound);
    const auto different_range = LeftClosedRange(left_bound, right_bound + 1);

    EXPECT_THAT(range, Eq(same_range));
    EXPECT_THAT(range, Ne(different_range));
}

TEST(LeftClosedRange, ThrowsIfRightBoundSmallerThanLeftBound) {
    constexpr auto left_bound = 4;
    constexpr auto right_bound = left_bound - 1;

    EXPECT_THROW(LeftClosedRange(left_bound, right_bound), std::invalid_argument);
}

TEST(LeftClosedRange, CanCalculateOverlayWithOtherInterval) {
    const auto interval = LeftClosedRange(4, 7);
    const auto other_interval = LeftClosedRange(6, 8);

    const auto expected_overlay = LeftClosedRange(6, 7);
    EXPECT_THAT(interval.get_overlay(other_interval).value(), Eq(expected_overlay));
}

TEST(LeftClosedRange, ReturnsNoValueIfNoOverlay) {
    const auto interval = LeftClosedRange(4, 7);
    const auto other_interval = LeftClosedRange(7, 8);

    EXPECT_FALSE(interval.get_overlay(other_interval).has_value());
}

TEST(LeftClosedRange, IsIterable) {
    const auto interval = LeftClosedRange(4, 7);

    auto values = std::vector<int>{};

    for (const auto value: interval) { values.push_back(value); }

    const auto expected = std::vector{4, 5, 6};

    EXPECT_THAT(values, Eq(expected));
}

TEST(Chain, ChainsIntervals) {
    const auto intervals = std::vector{LeftClosedRange(0, 2), LeftClosedRange(4, 8)};
    const auto chained_intervals = chain(intervals);

    const auto expected = std::vector{LeftClosedRange(0, 2), LeftClosedRange(2, 6)};
    EXPECT_THAT(chained_intervals, Eq(expected));
}

TEST(Chain, StartsAlwaysAt0) {
    const auto intervals = std::vector{LeftClosedRange(2, 6)};
    const auto chained_intervals = chain(intervals);
    EXPECT_THAT(chained_intervals[0].get_left_bound(), Eq(0));
}