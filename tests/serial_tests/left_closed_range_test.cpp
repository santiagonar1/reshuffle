#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <left_closed_range.hpp>

using namespace reshuffle::internal;

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