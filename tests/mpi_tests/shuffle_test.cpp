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
    const auto global_values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, value) : std::vector<int>{};

    const auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    EXPECT_THAT(my_values, Eq(std::vector(_min_elements_per_rank, value)));
}

TEST_F(Shuffle, WorksForDifferentDatatypes) {
    constexpr double value = 42.1;
    const auto global_values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank, value)
                                         : std::vector<double>{};

    const auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    EXPECT_THAT(my_values, Eq(std::vector(_min_elements_per_rank, value)));
}

TEST_F(Shuffle, GivesByDefaultRemainingElementsToLastRank) {
    constexpr int value = 42;
    const auto global_values = is_root() ? std::vector(_num_ranks * _min_elements_per_rank + 1, value)
                                         : std::vector<int>{};

    const auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    if (not is_last()) {
        EXPECT_THAT(my_values, Eq(std::vector(_min_elements_per_rank, value)));
    } else {
        EXPECT_THAT(my_values, Eq(std::vector(_min_elements_per_rank + 1, value)));
    }
}

TEST_F(Shuffle, WorksIfEachRankHasData) {
    std::vector<int> values_before(_min_elements_per_rank, _rank);

    const auto my_values = reshuffle::shuffle(values_before, MPI_COMM_WORLD);
    EXPECT_THAT(my_values, Eq(values_before));
}