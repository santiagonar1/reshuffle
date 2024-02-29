#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <mpi.h>
#include <vector>
#include <reshuffle.hpp>

using ::testing::Eq;

class Shuffle : public testing::Test {
protected:
    int _num_ranks{};
    int _rank{};
    const int _min_elements_per_rank{10};

    Shuffle() {
        MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &_num_ranks);
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
    const auto values = std::vector(_min_elements_per_rank, _rank);

    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(values));
}


TEST_F(Shuffle, WorksIfSourceAndDestinyCommunicatorsAreDifferent) {
    constexpr int value = 42;
    const auto values = std::vector(_min_elements_per_rank * _num_ranks, value);

    MPI_Group world_group;
    MPI_Comm_group(MPI_COMM_WORLD, &world_group);

    auto ranks = std::vector<int>{0};
    MPI_Group group_rank_0;
    MPI_Group_incl(world_group, 1, ranks.data(), &group_rank_0);

    MPI_Comm comm_rank_0;
    MPI_Comm_create(MPI_COMM_WORLD, group_rank_0, &comm_rank_0);

    const auto new_values = reshuffle::shuffle(values, comm_rank_0, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, value)));
}