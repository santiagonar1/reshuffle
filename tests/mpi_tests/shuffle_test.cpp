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
};

TEST_F(Shuffle, SplitsDataEquallyAmongRanks) {
    std::vector<int> global_values{};
    if (is_root()) {
        global_values.resize(_num_ranks * _min_elements_per_rank);
    }

    auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    EXPECT_THAT(my_values.size(), Eq(_min_elements_per_rank));
}

TEST_F(Shuffle, WorksForDifferentDatatypes) {
    std::vector<double> global_values{};
    if (is_root()) {
        global_values.resize(_num_ranks * _min_elements_per_rank);
    }

    auto my_values = reshuffle::shuffle(global_values, MPI_COMM_WORLD);
    EXPECT_THAT(_min_elements_per_rank, Eq(my_values.size()));
}