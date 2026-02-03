#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <utils.hpp>

using namespace reshuffle::internal;

using testing::Eq;

TEST(Unzip, TakesAVectorOfTuplesAndReturnsAnArrayOfVectors) {
    const auto zipped_values = std::vector{std::make_tuple(1, 2), std::make_tuple(3, 4)};
    EXPECT_THAT(unzip(zipped_values), Eq(std::array{std::vector{1, 3}, std::vector{2, 4}}));
}

TEST(Unzip, WorksWithAnyTupleLikeType) {
    const auto zipped_values = std::vector{std::array{1, 2}, std::array{3, 4}};
    EXPECT_THAT(unzip(zipped_values), Eq(std::array{std::vector{1, 3}, std::vector{2, 4}}));
}

TEST(RemoveDuplicates, RemovesDuplicatesFromAVector) {
    const auto values = std::vector{1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5};
    const auto expected = std::vector{1, 2, 3, 4, 5, 6};
    EXPECT_THAT(remove_duplicates(values), Eq(expected));
}

TEST(Find, ReturnsValueAssociatedWithKey) {
    const auto map = std::map<int, int>{{1, 2}, {3, 4}};

    EXPECT_THAT(find(map, 1).value(), Eq(2));
}

TEST(Find, ReturnsNulloptIfNoKeyWithThatValue) {
    const auto map = std::map<int, int>{{1, 2}, {3, 4}};
    EXPECT_FALSE(find(map, 5).has_value());
}

TEST(Sort, SortsAVector) {
    const auto values = std::vector{3, 1, 2};
    EXPECT_THAT(sort(values), Eq(std::vector{1, 2, 3}));
}

TEST(Drop, DropsOneElementFromArray) {
    constexpr auto values = std::array{0, 1, 2, 3};

    EXPECT_THAT(drop(values, 0), Eq(std::array{1, 2, 3}));
    EXPECT_THAT(drop(values, 1), Eq(std::array{0, 2, 3}));
}
