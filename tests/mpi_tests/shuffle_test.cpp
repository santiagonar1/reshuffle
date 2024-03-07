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
    MPI_Comm _comm_rank_0{};
    MPI_Comm _comm_rank_1{};
    const std::vector<int> _values{};

    Shuffle() : _values(_min_elements_per_rank, 42) {
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
    }

    [[nodiscard]] bool is_root() const {
        return _rank == 0;
    }

    [[nodiscard]] bool is_last() const {
        return _rank == _num_ranks - 1;
    }
};

TEST_F(Shuffle, SplitsDataEquallyAmongRanks) {
    constexpr int value = 42;
    const auto values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, value) : std::vector<int>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
}

TEST_F(Shuffle, WorksForDifferentDatatypes) {
    constexpr double value = 42.1;
    const auto values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, value)
                                  : std::vector<double>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
}

TEST_F(Shuffle, GivesByDefaultRemainingElementsToLastRank) {
    constexpr int value = 42;
    const auto values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank + 1, value)
                                  : std::vector<int>{};

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    if (is_last()) {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank + 1, value)));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
    }
}

TEST_F(Shuffle, WorksIfEachRankHasData) {
    const auto new_values = reshuffle::shuffle(_values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}


TEST_F(Shuffle, WorksIfSourceAndDestinyCommunicatorsAreDifferent) {
    constexpr int value = 42;
    const auto values = std::vector(_min_elements_per_rank * _num_ranks, value);

    const auto new_values = reshuffle::shuffle(values, _comm_rank_0, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
}

TEST_F(Shuffle, ThrowsIfCommunicatorsDoNotContainRootProcessor) {
    EXPECT_THROW(reshuffle::shuffle(_values, _comm_rank_1, MPI_COMM_WORLD), std::invalid_argument);
}

TEST_F(Shuffle, WorksWithContiguousContainers) {
    constexpr int value = 42;
    constexpr int num_values_per_rank = 10;
    auto values = std::array<int, num_values_per_rank>{};
    values.fill(value);

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(num_values_per_rank, value)));
}

TEST_F(Shuffle, WorksWithAnyIterableContainer) {
    constexpr int value = 42;
    constexpr int num_values_per_rank = 10;
    auto values = std::list(num_values_per_rank, value);

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(num_values_per_rank, value)));
}