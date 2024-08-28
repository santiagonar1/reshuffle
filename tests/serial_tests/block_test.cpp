#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block.hpp>

using testing::Eq;

TEST(Block, IsALeftClosedInterval) {
    constexpr int left_bound{0};
    constexpr int right_bound{4};

    const auto block = reshuffle::Block{left_bound, right_bound};

    EXPECT_TRUE(block.contains(left_bound));
    EXPECT_FALSE(block.contains(right_bound));
}