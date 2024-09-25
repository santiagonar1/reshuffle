#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utils.hpp>

using testing::Eq;
using namespace reshuffle::internal;

TEST(Combine, CalculatesTheCartesianProductOfTwoVectors) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{3, 4};

    EXPECT_THAT(combine(v1, v2), Eq(std::vector{std::pair{1, 3}, std::pair{2, 3}, std::pair{1, 4},
                                                std::pair{2, 4}}));
}
