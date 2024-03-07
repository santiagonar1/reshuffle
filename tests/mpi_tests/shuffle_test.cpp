#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mpi.h>
#include <vector>

#include <array>
#include <list>
#include <reshuffle.hpp>

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
    constexpr double value = 42.1;
    const auto values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, value)
                                  : std::vector<double>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
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
    auto values = std::list(_min_elements_per_rank, _value);

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
}