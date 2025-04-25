#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <cartesian_view.hpp>

using namespace reshuffle::views;

using testing::Eq;

TEST(CartesianView, CreatesACartesianProduct) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{'a', 'b'};

    // Expected result as a vector of tuples
    std::vector<std::tuple<int, char>> expected = {{1, 'a'}, {1, 'b'}, {2, 'a'}, {2, 'b'}};

    auto product = cartesian_product(v1, v2);
    auto result = std::vector<std::tuple<int, char>>();
    for (const auto &pair: product) { result.emplace_back(pair); }

    EXPECT_THAT(result, Eq(expected));
}

TEST(CartesianView, CartesianProductWithEmtpySetIsEmtpy) {
    const auto v = std::vector{1, 2};
    constexpr auto empty = std::vector<char>{};

    const auto product = cartesian_product(v, empty);

    EXPECT_TRUE(product.empty());
}

TEST(CartesianView, GetsTheCartesianProductOfVectorsInsideOfArray) {
    const auto v1 = std::vector{1, 2};
    const auto v2 = std::vector{3, 4};
    const auto array = std::array{v1, v2};

    // Expected result as a vector of tuples
    const std::vector<std::tuple<int, int>> expected = {{1, 3}, {1, 4}, {2, 3}, {2, 4}};

    auto product = cartesian_product(array);
    auto result = std::vector<std::tuple<int, int>>();
    for (const auto &pair: product) { result.emplace_back(pair); }

    EXPECT_THAT(result, Eq(expected));
}

TEST(CartesianView, CartesianProductOfArrayWithOnlyOneVectorIsVectorOfTuple) {
    const auto v1 = std::vector{1, 2};
    const auto array = std::array{v1};

    // Expected result as a vector of tuples
    const std::vector<std::tuple<int>> expected = {{1}, {2}};

    auto product = cartesian_product(array);
    auto result = std::vector<std::tuple<int>>();
    for (const auto &pair: product) { result.emplace_back(pair); }

    EXPECT_THAT(result, Eq(expected));
}