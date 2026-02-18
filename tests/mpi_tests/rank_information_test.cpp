#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi_comm_utils.hpp>
#include <rank_information.hpp>

#include "context_creation.hpp"

using namespace reshuffle;
using namespace reshuffle::mpi;
using namespace reshuffle::internal;

using testing::Eq;

TEST(RankInformation, StoresRankInformationForInitialAndFinalContext) {
    const auto sub_comm_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0}).value_or(MPI_COMM_NULL);
    const auto inter_communicator = InterCommunicator(sub_comm_0, MPI_COMM_WORLD);
    const auto initial_processor_grid = ProcessorGrid{1};
    const auto final_processor_grid = ProcessorGrid{2};

    const auto rank_information =
            RankInformation{inter_communicator, initial_processor_grid, final_processor_grid};

    if (const auto rank = get_rank_id(MPI_COMM_WORLD).value(); rank == 0) {
        EXPECT_THAT(rank_information.get_initial_id(), Eq(0));
        EXPECT_THAT(rank_information.get_inter_communicator_id(), Eq(0));
        EXPECT_THAT(rank_information.get_final_id(), Eq(0));
        EXPECT_THAT(rank_information.get_initial_rank_coordinates(), Eq(Coordinates{0}));
        EXPECT_THAT(rank_information.get_final_rank_coordinates(), Eq(Coordinates{0}));
    } else {
        EXPECT_THAT(rank_information.get_initial_id(), Eq(reshuffle::INVALID_RANK_ID));
        EXPECT_THAT(rank_information.get_inter_communicator_id(), Eq(1));
        EXPECT_THAT(rank_information.get_final_id(), Eq(1));
        EXPECT_THAT(rank_information.get_initial_rank_coordinates(),
                    Eq(Coordinates{reshuffle::INVALID_RANK_ID}));
        EXPECT_THAT(rank_information.get_final_rank_coordinates(), Eq(Coordinates{1}));
    }
}