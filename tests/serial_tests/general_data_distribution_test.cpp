#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <general_data_distribution.hpp>

#include <array>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;
using testing::UnorderedElementsAreArray;

[[nodiscard]] auto as_array(const std::pair<Interval, Interval> &pairs) -> std::array<Interval, 2>;


TEST(GeneralDataDistribution, IsCreatedWithMakeFunction) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto distribution = GeneralDataDistribution<1>::make(global_mapping, mapping_0, 0);
    EXPECT_TRUE(distribution.has_value());
}

TEST(GeneralDataDistribution, CreatesAProcessorGrid) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}, Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{0, 2}, Interval{2, 4}}};
    constexpr auto num_ranks = 100;

    const auto global_mapping = GlobalMapping<2>{{0, intervals_0}, {num_ranks - 1, intervals_1}};

    const auto mapping_0 = std::map<IntervalId, Coordinates<2>>{{0, {0, 0}}};

    const auto distribution =
            GeneralDataDistribution<2>::make(global_mapping, mapping_0, 0).value();

    EXPECT_THAT(distribution.get_processor_grid(), Eq(ProcessorGrid{num_ranks, 1}));
}

TEST(GeneralDataDistribution, CreatesAGridLayout) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};
    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto distribution =
            GeneralDataDistribution<1>::make(global_mapping, mapping_0, 0).value();
    const auto grid_layout = distribution.get_grid_layout();

    const auto expected_blocks = std::vector{Block{{0, 2}, 1}, Block{{2, 4}, 0}};
    EXPECT_THAT(grid_layout.get_blocks(), Eq(std::array{expected_blocks}));
}

TEST(GeneralDataDistribution, CanBeCloned) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};
    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto distribution =
            GeneralDataDistribution<1>::make(global_mapping, mapping_0, 0).value();

    const auto clone = distribution.clone();
    EXPECT_THAT(*clone, Eq(distribution));
}

TEST(GeneralDataDistribution, DisjointIntervalsAreCaught) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto disjoint_interval = std::vector{MultidimensionalInterval{Interval{3, 4}}};

    const auto disjoint_mapping = GlobalMapping<1>{{0, disjoint_interval}, {1, intervals_1}};
    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto distribution = GeneralDataDistribution<1>::make(disjoint_mapping, mapping_0, 0);
    EXPECT_FALSE(distribution.has_value());
}

TEST(GeneralDataDistribution, OverlappingIntervalsAreCaught) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto overlapping_interval = std::vector{MultidimensionalInterval{Interval{1, 4}}};

    const auto overlapping_mapping = GlobalMapping<1>{{0, overlapping_interval}, {1, intervals_1}};
    const auto mapping_0 = std::map<IntervalId, Coordinates<1>>{{0, {0}}};

    const auto distribution = GeneralDataDistribution<1>::make(overlapping_mapping, mapping_0, 0);
    EXPECT_FALSE(distribution.has_value());
}

TEST(GeneralDataDistribution, AllLocalBlocksShouldBeMapped) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto mapping_0_missing_blocks = std::map<IntervalId, Coordinates<1>>{};
    const auto distribution =
            GeneralDataDistribution<1>::make(global_mapping, mapping_0_missing_blocks, 0);
    EXPECT_FALSE(distribution.has_value());
}

TEST(GeneralDataDistribution, AllocatingMoreLocalBlocksThanAssignedIsWrong) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto mapping_0_more_blocks = std::map<IntervalId, Coordinates<1>>{{0, {0}}, {1, {1}}};
    const auto distribution =
            GeneralDataDistribution<1>::make(global_mapping, mapping_0_more_blocks, 0);
    EXPECT_FALSE(distribution.has_value());
}

TEST(GeneralDataDistribution, WorksIfCalledFromARankNotInGlobalMapping) {
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{0, 2}}};
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{2, 4}}};

    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto mapping_2 = std::map<IntervalId, Coordinates<1>>{};
    const auto distribution = GeneralDataDistribution<1>::make(global_mapping, mapping_2, 2);
    EXPECT_TRUE(distribution.has_value());
}

TEST(FindProblematicIntervals, ReturnFirstPairOfProblematicIntervals) {
    const auto intervals = std::vector{Interval{0, 1}, Interval{1, 3}, Interval{2, 6}};

    const auto expected = std::array{Interval{1, 3}, Interval{2, 6}};
    const auto result = as_array(find_problematic_intervals(intervals).value());
    EXPECT_THAT(result, UnorderedElementsAreArray(expected));
}

TEST(FindProblematicIntervals, DisjointedIntervalsAreProblematic) {
    const auto disjointed_intervals = std::vector{Interval{0, 1}, Interval{2, 6}};

    const auto expected = std::array{Interval{0, 1}, Interval{2, 6}};
    const auto result = as_array(find_problematic_intervals(disjointed_intervals).value());
    EXPECT_THAT(result, UnorderedElementsAreArray(expected));
}

TEST(FindProblematicIntervals, OverlappingIntervalsAreProblematic) {
    const auto overlapping_intervals = std::vector{Interval{0, 1}, Interval{0, 6}};

    const auto expected = std::array{Interval{0, 1}, Interval{0, 6}};
    const auto result = as_array(find_problematic_intervals(overlapping_intervals).value());
    EXPECT_THAT(result, UnorderedElementsAreArray(expected));
}

TEST(FindProblematicIntervals, ReturnsNulloptIfNoProblemsFound) {
    const auto nice_intervals = std::vector{Interval{0, 1}, Interval{1, 3}};
    EXPECT_FALSE(find_problematic_intervals(nice_intervals).has_value());
}

TEST(GetIntervals, ReturnsMultidimensionalIntervalsInGlobalMapping) {
    const auto intervals_0 = std::vector{MultidimensionalInterval{Interval{0, 2}},
                                         MultidimensionalInterval{Interval{1, 6}}};
    const auto intervals_1 = std::vector{MultidimensionalInterval{Interval{4, 6}},
                                         MultidimensionalInterval{Interval{7, 8}}};
    const auto global_mapping = GlobalMapping<1>{{0, intervals_0}, {1, intervals_1}};

    const auto expected =
            std::vector{intervals_0[0], intervals_0[1], intervals_1[0], intervals_1[1]};
    EXPECT_THAT(get_intervals(global_mapping), Eq(expected));
}

TEST(GetMaxRank, ReturnsTheMaxRankIdUsedAsKeyInGlobalMapping) {
    constexpr auto max_rank = 100;
    const auto global_mapping = GlobalMapping<1>{{0, {}}, {max_rank, {}}};

    EXPECT_THAT(get_max_rank(global_mapping), Eq(max_rank));
}

auto as_array(const std::pair<Interval, Interval> &pairs) -> std::array<Interval, 2> {
    return std::array{pairs.first, pairs.second};
}