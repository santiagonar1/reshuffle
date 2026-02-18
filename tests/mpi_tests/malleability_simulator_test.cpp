#include "malleability_simulator.hpp"
#include "reshuffle.hpp"


#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace reshuffle::mpi;

using testing::Eq;

TEST(MalleabilitySimulator, NeedsABaseCommToBeCreated) {
    const auto base_comm = MPI_COMM_WORLD;
    const auto initial_comm = base_comm;

    auto simulator = MalleabilitySimulator{base_comm, initial_comm};
    const auto num_ranks = get_num_ranks(base_comm).value();

    EXPECT_TRUE(simulator.request_adaptation(num_ranks).has_value());
}

TEST(RequestAdaptation, ReturnsRankStatus) {
    const auto base_comm = MPI_COMM_WORLD;
    const auto initial_comm = base_comm;

    auto simulator = MalleabilitySimulator{base_comm, initial_comm};
    const auto num_ranks = get_num_ranks(base_comm).value();

    EXPECT_THAT(simulator.request_adaptation(num_ranks).value().second, Eq(RankStatus::STAYING));
}

TEST(RequestAdaptation, IndicatesIfRankIsJoining) {
    const auto all_ranks_comm = MPI_COMM_WORLD;
    const auto only_rank_0_comm =
            get_sub_comm(all_ranks_comm, std::vector{0}).value_or(MPI_COMM_NULL);

    auto simulator = MalleabilitySimulator{all_ranks_comm, only_rank_0_comm};
    const auto num_ranks = get_num_ranks(all_ranks_comm).value();

    const auto [_, rank_status] = simulator.request_adaptation(num_ranks).value();

    if (is_root(all_ranks_comm)) {
        EXPECT_THAT(rank_status, Eq(RankStatus::STAYING));
    } else {
        EXPECT_THAT(rank_status, Eq(RankStatus::JOINING));
    }
}

TEST(RequestAdaptation, IndicatesIfRankIsLeaving) {
    const auto all_ranks_comm = MPI_COMM_WORLD;

    auto simulator = MalleabilitySimulator{all_ranks_comm, all_ranks_comm};

    const auto [_, rank_status] = simulator.request_adaptation(1).value();

    if (is_root(all_ranks_comm)) {
        EXPECT_THAT(rank_status, Eq(RankStatus::STAYING));
    } else {
        EXPECT_THAT(rank_status, Eq(RankStatus::LEAVING));
    }
}

TEST(RequestAdaptation, IndicatesIfRankIsInactive) {
    const auto all_ranks_comm = MPI_COMM_WORLD;
    const auto only_rank_0_comm =
            get_sub_comm(all_ranks_comm, std::vector{0}).value_or(MPI_COMM_NULL);

    auto simulator = MalleabilitySimulator{all_ranks_comm, only_rank_0_comm};

    const auto [_, rank_status] = simulator.request_adaptation(1).value();

    if (is_root(all_ranks_comm)) {
        EXPECT_THAT(rank_status, Eq(RankStatus::STAYING));
    } else {
        EXPECT_THAT(rank_status, Eq(RankStatus::INACTIVE));
    }
}