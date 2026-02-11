#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <block_wise.hpp>
#include <context.hpp>

using namespace reshuffle;
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