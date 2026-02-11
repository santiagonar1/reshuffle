#include "mpi_utils.hpp"


#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_wise.hpp>
#include <context.hpp>

using namespace reshuffle;
using namespace reshuffle::mpi;
using namespace reshuffle::internal;

TEST(Contexts, CanBeCompared) {
    constexpr auto num_global_values = 256;
    const auto processor_grid = ProcessorGrid{3};

    const auto context =
            Context{BlockWise{Dimensions{num_global_values}, processor_grid}, MPI_COMM_WORLD};
    const auto same_context =
            Context{BlockWise{Dimensions{num_global_values}, processor_grid}, MPI_COMM_WORLD};

    EXPECT_TRUE(context == same_context);
}

TEST(Context, TwoContextsWithDifferentCommsAreDifferent) {
    constexpr auto num_global_values = 256;
    const auto processor_grid = ProcessorGrid{3};

    const auto comm_o = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto comm_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto context = Context{BlockWise{Dimensions{num_global_values}, processor_grid}, comm_o};
    const auto different_context =
            Context{BlockWise{Dimensions{num_global_values}, processor_grid}, comm_1};

    EXPECT_FALSE(context == different_context);
}

TEST(Context, TwoContextsWithDifferentDistributionsAreDifferent) {
    constexpr auto num_global_values = 256;
    const auto processor_grid = ProcessorGrid{3};

    const auto comm_o = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto comm_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto context = Context{BlockWise{Dimensions{num_global_values}, processor_grid}, comm_o};
    const auto different_context =
            Context{BlockWise{Dimensions{num_global_values + 1}, processor_grid}, comm_1};

    EXPECT_FALSE(context == different_context);
}