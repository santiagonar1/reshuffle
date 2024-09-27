#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block.hpp>

using testing::Eq;

TEST(Block, IsALeftClosedInterval) {
    static_assert(std::is_same_v<reshuffle::Block, reshuffle::internal::LeftClosedRange>,
                  "block should be a left closed range");
}