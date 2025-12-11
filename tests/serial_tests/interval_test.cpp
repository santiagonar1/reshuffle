#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <interval.hpp>

using namespace reshuffle;

TEST(AnInterval, IsALeftClosedRange) {
    static_assert(std::is_same_v<Interval, LeftClosedRange>, "An Interval is a LeftClosedRange");
}