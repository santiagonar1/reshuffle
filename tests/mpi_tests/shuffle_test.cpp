#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi.h>
#include <vector>

#include <array>
#include <list>
#include <reshuffle.hpp>

using testing::Eq;
using testing::ThrowsMessage;

class Shuffle : public testing::Test {
protected:
    int _num_ranks{};
    static constexpr int _min_elements_per_rank{10};
    static constexpr int _value{42};
    int _total_num_values{};
    MPI_Comm _comm_rank_0{MPI_COMM_NULL};
    MPI_Comm _comm_rank_1{MPI_COMM_NULL};
    const std::vector<int> _values{};
    std::vector<int> _values_only_in_root{};

    Shuffle() : _values(_min_elements_per_rank, _value) {
        MPI_Comm_rank(MPI_COMM_WORLD, &_rank);
        MPI_Comm_size(MPI_COMM_WORLD, &_num_ranks);
        _total_num_values = _min_elements_per_rank * _num_ranks;

        MPI_Group world_group;
        MPI_Comm_group(MPI_COMM_WORLD, &world_group);

        auto ranks = std::vector<int>{0};

        if (is_root()) {
            MPI_Group group_rank_0;
            MPI_Group_incl(world_group, 1, ranks.data(), &group_rank_0);

            MPI_Comm_create(MPI_COMM_WORLD, group_rank_0, &_comm_rank_0);

            _values_only_in_root = std::vector(_total_num_values, _value);
        } else {
            MPI_Group group_rank_1;
            ranks[0] = 1;
            MPI_Group_incl(world_group, 1, ranks.data(), &group_rank_1);
            MPI_Comm_create(MPI_COMM_WORLD, group_rank_1, &_comm_rank_1);
        }
    }

    [[nodiscard]] bool is_root() const { return _rank == 0; }

    [[nodiscard]] bool is_last() const { return _rank == _num_ranks - 1; }

private:
    int _rank{};
};

TEST_F(Shuffle, In1DWorksWithAnyIterableContainer) {
    const auto values = std::list(_values_only_in_root.begin(), _values_only_in_root.end());
    const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, In1DSplitsDataEquallyAmongRanksByDefault) {
    const auto new_values = reshuffle::shuffle(_values_only_in_root, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, IfNumValuesNoDivisibleGivesAdditionalDataToInitialRanks) {
    if (is_root()) { _values_only_in_root.push_back(_value); }

    const auto new_values = reshuffle::shuffle(_values_only_in_root, MPI_COMM_WORLD);
    if (is_last()) {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank, _value)));
    } else {
        EXPECT_THAT(new_values, Eq(std::vector(_min_elements_per_rank + 1, _value)));
    }
}

TEST_F(Shuffle, CanBePassedADataDistribution) {
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(_total_num_values, 1);

    const auto new_values =
            reshuffle::shuffle(_values, MPI_COMM_WORLD, old_distribution, new_distribution);

    if (is_root()) {
        EXPECT_THAT(new_values, Eq(_values_only_in_root));
    } else {
        EXPECT_TRUE(new_values.empty());
    }
}

TEST_F(Shuffle, ThrowsIfDataDistributionsDoNotHaveSameNumberOfValues) {
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);
    const auto new_distribution =
            reshuffle::make_block_wise(old_distribution.get_num_values() + 1, 1);

    EXPECT_THROW(auto new_values = reshuffle::shuffle(_values, MPI_COMM_WORLD, old_distribution,
                                                      new_distribution),
                 std::invalid_argument);
}

TEST_F(Shuffle, ThrowsIfNumberOfValuesPassedInconsistentWithCurrentDistribution) {
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(_total_num_values, 1);

    const auto shuffle_call = [old_distribution, new_distribution] {
        auto new_values = reshuffle::shuffle(std::vector<int>{}, MPI_COMM_WORLD, old_distribution,
                                             new_distribution);
    };

    const auto shuffle_different_comm = [old_distribution, new_distribution, this] {
        auto new_values = reshuffle::shuffle(std::vector<int>{}, MPI_COMM_WORLD, _comm_rank_0,
                                             old_distribution, new_distribution);
    };

    EXPECT_THAT(shuffle_call,
                ThrowsMessage<std::invalid_argument>(
                        "Number of values provided not consistent with current distribution"));

    EXPECT_THAT(shuffle_different_comm,
                ThrowsMessage<std::invalid_argument>(
                        "Number of values provided not consistent with current distribution"));
}

TEST_F(Shuffle, WorksIn2DButDataDistributionMustBeUsed) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_values_x = 4;
    constexpr int num_values_y = 10;

    auto m = is_root() ? Matrix(num_values_y, std::vector(num_values_x, 0)) : Matrix();
    const auto old_distribution = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                             reshuffle::make_block_wise(num_values_y, 1)};
    const auto new_distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                             reshuffle::make_block_wise(num_values_y, 1)};

    m = reshuffle::shuffle(m, MPI_COMM_WORLD, old_distribution, new_distribution);

    EXPECT_THAT(m.size(), Eq(num_values_y));
    EXPECT_THAT(m[0].size(), Eq(num_values_x / 2));
}

TEST_F(Shuffle, ThrowsIn2DIfDataDistributionsDoNotHaveSameNumberOfValuesOnEachDimension) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_values_x = 4;
    constexpr int num_values_y = 10;

    auto m = is_root() ? Matrix(num_values_y, std::vector(num_values_x, 0)) : Matrix();
    const auto old_distribution = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                             reshuffle::make_block_wise(num_values_y, 1)};
    const auto new_distribution = std::array{reshuffle::make_block_wise(num_values_x + 1, 2),
                                             reshuffle::make_block_wise(num_values_y, 1)};

    EXPECT_THROW(m = reshuffle::shuffle(m, MPI_COMM_WORLD, old_distribution, new_distribution),
                 std::invalid_argument);
}

TEST_F(Shuffle, CanBePassedDifferentCommunicators) {
    const auto new_values = reshuffle::shuffle(_values_only_in_root, _comm_rank_0, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, WorksIn2DWithDifferentCommunicators) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_values_x = 4;
    constexpr int num_values_y = 10;

    auto m = is_root() ? Matrix(num_values_y, std::vector(num_values_x, 0)) : Matrix();
    const auto old_distribution = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                             reshuffle::make_block_wise(num_values_y, 1)};
    const auto new_distribution = std::array{reshuffle::make_block_wise(num_values_x, 2),
                                             reshuffle::make_block_wise(num_values_y, 1)};

    m = reshuffle::shuffle(m, _comm_rank_0, MPI_COMM_WORLD, old_distribution, new_distribution);

    EXPECT_THAT(m.size(), Eq(num_values_y));
    EXPECT_THAT(m[0].size(), Eq(num_values_x / 2));
}

TEST_F(Shuffle, CanBePassedADataDistributionAndDifferentCommunicators) {
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(_total_num_values, 1);

    const auto new_values = reshuffle::shuffle(_values, MPI_COMM_WORLD, _comm_rank_0,
                                               old_distribution, new_distribution);

    if (is_root()) {
        EXPECT_THAT(new_values, Eq(_values_only_in_root));
    } else {
        EXPECT_TRUE(new_values.empty());
    }
}

TEST_F(Shuffle, WorksWithAnyIterableContainerAndDataDistribution) {
    const auto values = std::list(_values_only_in_root.begin(), _values_only_in_root.end());
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, 1);
    const auto new_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);

    const auto new_values =
            reshuffle::shuffle(values, MPI_COMM_WORLD, old_distribution, new_distribution);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, WorksWithAnyIterableContainerAndDataDistributionAndDifferentCommunicators) {
    const auto values = std::list(_values_only_in_root.begin(), _values_only_in_root.end());
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, 1);
    const auto new_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);

    const auto new_values = reshuffle::shuffle(values, _comm_rank_0, MPI_COMM_WORLD,
                                               old_distribution, new_distribution);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, WorksIfEachRankHasData) {
    const auto new_values = reshuffle::shuffle(_values, MPI_COMM_WORLD);
    EXPECT_THAT(new_values, Eq(_values));
}

TEST_F(Shuffle, ThrowsIfDataDistributionsDoNotHaveSameNumberOfValuesDifferentCommunicators) {
    const auto old_distribution = reshuffle::make_block_wise(_total_num_values, _num_ranks);
    const auto new_distribution =
            reshuffle::make_block_wise(old_distribution.get_num_values() + 1, 1);

    EXPECT_THROW(auto new_values =
                         reshuffle::shuffle(_values_only_in_root, _comm_rank_0, MPI_COMM_WORLD,
                                            old_distribution, new_distribution),
                 std::invalid_argument);
}

TEST_F(Shuffle,
       ThrowsIn2DIfDataDistributionsDoNotHaveSameNumberOfValuesOnEachDimensionDifferentCommunicators) {
    using Matrix = std::vector<std::vector<int>>;
    constexpr int num_columns = 4;
    constexpr int num_rows = 10;

    auto m = is_root() ? Matrix(num_rows, std::vector(num_columns, 0)) : Matrix();
    const auto old_distribution = std::array{reshuffle::make_block_wise(num_columns, 1),
                                             reshuffle::make_block_wise(num_rows, 1)};
    const auto new_distribution = std::array{reshuffle::make_block_wise(num_columns + 1, 2),
                                             reshuffle::make_block_wise(num_rows, 1)};

    EXPECT_THROW(m = reshuffle::shuffle(m, _comm_rank_0, MPI_COMM_WORLD, old_distribution,
                                        new_distribution),
                 std::invalid_argument);
}

TEST_F(Shuffle, WorksMerginAfter2DVerticalSplitting) {
    using Matrix = std::vector<std::vector<int>>;

    constexpr int num_rows = 2;
    constexpr int num_columns = 2;

    const auto original_matrix = is_root() ? Matrix{{0}, {2}} : Matrix{{1}, {3}};

    const auto old_distribution = std::array{reshuffle::make_block_wise(num_columns, 2),
                                             reshuffle::make_block_wise(num_rows, 1)};
    const auto new_distribution = std::array{reshuffle::make_block_wise(num_columns, 1),
                                             reshuffle::make_block_wise(num_rows, 1)};

    const auto matrix =
            reshuffle::shuffle(original_matrix, MPI_COMM_WORLD, old_distribution, new_distribution);

    if (is_root()) EXPECT_THAT(matrix, Eq(Matrix{{0, 1}, {2, 3}}));
}

TEST_F(Shuffle, Works2DVerticalSplitting) {
    using Matrix = std::vector<std::vector<int>>;
    using DataDistribution2D = std::array<reshuffle::BlockCyclic, 2>;

    constexpr int num_rows = 2;
    constexpr int num_columns = 2;

    const auto original_matrix = is_root() ? Matrix{{0, 1}, {2, 3}} : Matrix{};
    const std::vector<DataDistribution2D> distributions = {
            {reshuffle::make_block_wise(num_columns, 1), reshuffle::make_block_wise(num_rows, 1)},
            {reshuffle::make_block_wise(num_columns, 2), reshuffle::make_block_wise(num_rows, 1)}};

    auto matrix = original_matrix;
    for (int i = 1; i < distributions.size(); ++i) {
        matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, distributions[i - 1], distributions[i]);
    }

    if (is_root()) EXPECT_THAT(matrix, Eq(Matrix{{0}, {2}}));
}

TEST_F(Shuffle, Works2DFromVerticalToHorizontalSplitting) {
    using Matrix = std::vector<std::vector<int>>;
    using DataDistribution2D = std::array<reshuffle::BlockCyclic, 2>;

    constexpr int num_rows = 2;
    constexpr int num_columns = 2;

    const auto original_matrix = is_root() ? Matrix{{0}, {2}} : Matrix{{1}, {3}};
    const std::vector<DataDistribution2D> distributions = {
            {reshuffle::make_block_wise(num_columns, 2), reshuffle::make_block_wise(num_rows, 1)},
            {reshuffle::make_block_wise(num_columns, 1), reshuffle::make_block_wise(num_rows, 2)}};

    auto matrix = original_matrix;
    for (int i = 1; i < distributions.size(); ++i) {
        matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, distributions[i - 1], distributions[i]);
    }

    if (is_root()) EXPECT_THAT(matrix, Eq(Matrix{{0, 1}}));
}
