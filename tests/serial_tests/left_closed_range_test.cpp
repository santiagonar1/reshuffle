#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <left_closed_range.hpp>

using namespace reshuffle;
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

    const auto values = std::vector<int>{interval.begin(), interval.end()};

    const auto expected = std::vector{4, 5, 6};

    EXPECT_THAT(values, Eq(expected));
}

TEST(IsContiguous, TwoLeftClosedRangeAreContiguousIfOneAfterTheOther) {
    const auto first = LeftClosedRange{4, 7};
    const auto contiguous = LeftClosedRange{7, 8};
    const auto non_contiguous = LeftClosedRange{10, 12};

    EXPECT_TRUE(first.is_contiguous(contiguous));
    EXPECT_FALSE(first.is_contiguous(non_contiguous));
}

TEST(IsContiguous, OverlappingRangesAreNotContiguous) {
    const auto first = LeftClosedRange{4, 7};
    const auto overlapping = LeftClosedRange{6, 8};

    EXPECT_FALSE(first.is_contiguous(overlapping));
}

TEST(IsConsiguous, OrderDoesNotMatters) {
    const auto first = LeftClosedRange{4, 7};
    const auto second = LeftClosedRange{7, 18};

    EXPECT_TRUE(first.is_contiguous(second));
    EXPECT_TRUE(second.is_contiguous(first));
}

TEST(AreContiguous, CanCheckVectorOfIntervals) {
    const auto contiguous = std::vector{LeftClosedRange{4, 7}, LeftClosedRange{7, 10}};
    const auto non_contiguous = std::vector{LeftClosedRange{4, 7}, LeftClosedRange{10, 12}};

    EXPECT_TRUE(are_contiguous(contiguous));
    EXPECT_FALSE(are_contiguous(non_contiguous));
}

TEST(AreContiguous, ForVectorsOrderDoesNotMatter) {
    const auto contiguous = std::vector{LeftClosedRange{7, 10}, LeftClosedRange{4, 7}};

    EXPECT_TRUE(are_contiguous(contiguous));
}

TEST(AreContiguous, VectorsWithOverlappingIntervalsAreNotContiguous) {
    const auto overlapping = std::vector{LeftClosedRange{4, 7}, LeftClosedRange{6, 8}};

    EXPECT_FALSE(are_contiguous(overlapping));
}

TEST(AreContiguous, EmptyVectorIsContiguous) {
    constexpr auto empty = std::vector<LeftClosedRange>{};
    EXPECT_TRUE(are_contiguous(empty));
}

TEST(AreContiguous, VectorWithOneElementIsContigous) {
    const auto one_element = std::vector{LeftClosedRange{4, 7}};
    EXPECT_TRUE(are_contiguous(one_element));
}

TEST(IsDisjoint, ReturnsTrueIfSecondIntervalIsDisjoint) {
    const auto first = LeftClosedRange{4, 7};
    const auto disjoint = LeftClosedRange{10, 12};
    const auto overlapping = LeftClosedRange{6, 8};

    EXPECT_TRUE(first.is_disjoint(disjoint));
    EXPECT_FALSE(first.is_disjoint(overlapping));
}

TEST(IsDisjoint, OrderDoesNotMatter) {
    const auto first = LeftClosedRange{4, 7};
    const auto disjoint = LeftClosedRange{10, 12};

    EXPECT_TRUE(first.is_disjoint(disjoint));
    EXPECT_TRUE(disjoint.is_disjoint(first));
}

TEST(ToString, ConvertsALeftClosedRangeToString) {
    const auto interval = LeftClosedRange{4, 7};
    EXPECT_THAT(interval.to_string(), Eq("[4, 7)"));
}