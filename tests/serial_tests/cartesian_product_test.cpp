#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cartesian_product.hpp>

using namespace reshuffle::dev::internal;

using testing::Eq;


TEST(GetCartesianProduct, CalculatesTheCartesianProductOfVectorsInArray) {
    const auto v1 = std::vector{1, 2};
    const auto array = std::array{v1};

    const auto expected = std::vector<std::array<int, 1>>{{1}, {2}};
    EXPECT_THAT(get_cartesian_product(array), Eq(expected));
}

TEST(GetCartesianProduct, WorksIn2D) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{3, 4};
    const auto array = std::array{v1, v2};

    const auto expected = std::vector<std::array<int, 2>>{{1, 3}, {2, 3}, {1, 4}, {2, 4}};
    EXPECT_THAT(get_cartesian_product(array), Eq(expected));
}

TEST(GetCartesianProduct, WorksIn3D) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{3, 4};
    const auto v3 = std::vector{5, 6};
    const auto array = std::array{v1, v2, v3};

    const auto expected = std::vector<std::array<int, 3>>{
            {1, 3, 5}, {2, 3, 5}, {1, 4, 5}, {2, 4, 5}, {1, 3, 6}, {2, 3, 6}, {1, 4, 6}, {2, 4, 6}};
    EXPECT_THAT(get_cartesian_product(array), Eq(expected));
}

TEST(CartesianProduct, WithEmtpyVectorIsEmtpy) {
    const auto v1 = std::vector{1, 2};
    const auto empty = std::vector<int>{};
    const auto array = std::array{v1, empty};

    EXPECT_TRUE(get_cartesian_product(array).empty());
}