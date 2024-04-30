#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mpi.h>
#include <vector>

#include <array>
#include <list>
#include <reshuffle.hpp>

#include "my_type.hpp"

using ::testing::Eq;

class Shuffle : public testing::Test {
private:
    int _rank{};

protected:
    int _num_ranks{};
    static constexpr int _min_elements_per_rank{10};
    static constexpr int _value{42};
    MPI_Comm _comm_rank_0{};
    MPI_Comm _comm_rank_1{};
    const std::vector<int> _values{};
    std::vector<int> _values_only_in_root{};

    Shuffle() : _values(_min_elements_per_rank, _value) {
        MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &_num_ranks);

        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);

        auto ranks = std::vector<int>{0};
        MPI_Group group_rank_0;
        MPI_Group_incl(world_group, 1, ranks.data(), &group_rank_0);

        MPI_Comm_create(MPI_COMM_WORLD, group_rank_0, &_comm_rank_0);

        MPI_Group group_rank_1;
        ranks[0] = 1;
        MPI_Group_incl(world_group, 1, ranks.data(), &group_rank_1);
        MPI_Comm_create(MPI_COMM_WORLD, group_rank_1, &_comm_rank_1);

        _values_only_in_root = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, _value)
                                         : std::vector<int>{};
    }

    [[nodiscard]] bool is_root() const {
        return _rank == 0;
    }

    [[nodiscard]] bool is_last() const {
        return _rank == _num_ranks - 1;
    }
};

TEST_F(Shuffle, SplitsDataEquallyAmongRanks) {
    const auto new_values = reshuffle::shuffle(_values_only_in_root, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
}

TEST_F(Shuffle, WorksForDifferentDatatypes) {
    const std::vector<double> values(_values.begin(), _values.end());

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, static_cast<double>(_value))));
}

TEST_F(Shuffle, GivesByDefaultRemainingElementsToLastRank) {
    if (is_root()) {
        _values_only_in_root.push_back(_value);
    }

    const auto new_values = reshuffle::shuffle(_values_only_in_root, MPI_COMM_WORLD);
    if (is_last()) {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank + 1, _value)));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
    }
}

TEST_F(Shuffle, WorksIfEachRankHasData) {
    const auto new_values = reshuffle::shuffle(_values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}


TEST_F(Shuffle, WorksIfSourceAndDestinyCommunicatorsAreDifferent) {
    const auto new_values = reshuffle::shuffle(_values_only_in_root, _comm_rank_0, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
}

TEST_F(Shuffle, ThrowsIfCommunicatorsDoNotContainRootProcessor) {
    EXPECT_THROW(reshuffle::shuffle(_values, _comm_rank_1, MPI_COMM_WORLD), std::invalid_argument);
}

TEST_F(Shuffle, WorksWithContiguousContainers) {
    auto values = std::array<int, _min_elements_per_rank>{};
    values.fill(_value);

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
}

TEST_F(Shuffle, WorksWithAnyIterableContainer) {
    auto values = std::list(_values.begin(), _values.end());

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
}

TEST_F(Shuffle, WorksWithAggregateDatatype) {
    const auto values = is_root() ? std::vector<MyPOD>(_min_elements_per_rank * _num_ranks) : std::vector<MyPOD>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, MyPOD{})));
}

TEST_F(Shuffle, WorksWithNonAggregateDatatypeIfSerializableAndCreateMethodPresent) {
    const auto values = is_root() ? std::vector<NonAggregate>(_min_elements_per_rank * _num_ranks, NonAggregate(12))
                                  : std::vector<NonAggregate>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, NonAggregate(12))));
}

TEST_F(Shuffle, NonAggregateDefaultConstructibleDoesNotRequireCreate) {
    const auto values = is_root() ? std::vector<NonAggregateDefaultConstructible>(_min_elements_per_rank * _num_ranks)
                                  : std::vector<NonAggregateDefaultConstructible>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, NonAggregateDefaultConstructible())));
}

TEST_F(Shuffle, CanUseColoringToIndicateWhereValuesShouldBeStored) {
    const auto values = std::vector{1, 0, 1, 1};
    const auto coloring = std::vector{1, 0, 1, 1};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD, coloring);
    if (is_root()) {
        EXPECT_THAT(new_values, Eq(std::vector(2, 0)));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector(6, 1)));
    }
}

TEST_F(Shuffle, ColoringWorksWithDifferentCommunicators) {
    const auto values = std::vector{1, 0, 1, 1};
    const auto coloring = std::vector{1, 0, 1, 1};

    const auto new_values = reshuffle::shuffle(values, _comm_rank_0, MPI_COMM_WORLD, coloring);
    if (is_root()) {
        EXPECT_THAT(new_values, Eq(std::vector{0}));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector{1, 1, 1}));
    }
}

TEST_F(Shuffle, ColoringWorksWithNonAggregate) {
    const auto values = std::vector{NonAggregate(1), NonAggregate(0), NonAggregate(1), NonAggregate(1)};
    const auto coloring = std::vector{1, 0, 1, 1};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD, coloring);
    if (is_root()) {
        EXPECT_THAT(new_values, Eq(std::vector(2, NonAggregate(0))));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector(6, NonAggregate(1))));
    }
}

TEST_F(Shuffle, CanSplit2DContainers) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_rows = 10;
    constexpr int num_columns = 4;
    constexpr int num_values = num_rows * num_columns;

    auto m = is_root() ? Matrix(num_rows, std::vector(num_columns, 0)) : Matrix();
    auto coloring = is_root() ? std::vector(num_values, 1) : std::vector<int>{};
    auto subdomain_dimension = is_root() ? reshuffle::Dimensions2D{0, 0} : reshuffle::Dimensions2D{num_rows,
                                                                                                   num_columns};
    m = reshuffle::shuffle(m, MPI_COMM_WORLD, coloring, subdomain_dimension);

    if (is_root()) {
        EXPECT_THAT(m.size(), Eq(0));
    } else {
        EXPECT_THAT(m.size(), num_rows);
        EXPECT_THAT(m[0].size(), num_columns);
    }
}

TEST_F(Shuffle, CanSplit2DContainersWithDifferentCommunicators) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_rows = 10;
    constexpr int num_columns = 4;
    constexpr int num_values = num_rows * num_columns;

    auto m = is_root() ? Matrix(num_rows, std::vector(num_columns, 0)) : Matrix();
    auto coloring = is_root() ? std::vector(num_values, 1) : std::vector<int>{};
    auto subdomain_dimension = is_root() ? reshuffle::Dimensions2D{0, 0} : reshuffle::Dimensions2D{num_rows,
                                                                                                   num_columns};
    m = reshuffle::shuffle(m, _comm_rank_0, MPI_COMM_WORLD, coloring, subdomain_dimension);

    if (is_root()) {
        EXPECT_THAT(m.size(), Eq(0));
    } else {
        EXPECT_THAT(m.size(), num_rows);
        EXPECT_THAT(m[0].size(), num_columns);
    }
}