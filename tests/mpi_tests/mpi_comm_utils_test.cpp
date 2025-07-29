#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi_comm_utils.hpp>
#include <mpi_utils.hpp>

#include "../serial_tests/include/non_aggregate_data.hpp"
#include "aggregate_data.hpp"

using namespace reshuffle::internal;
using namespace reshuffle::mpi;

using ::testing::Eq;


TEST(AsyncSend, CanBeUsedToSendValuesFromOneRankToAnother) {
    auto values = is_root(MPI_COMM_WORLD) ? std::vector{1, 2, 3} : std::vector<int>(3);

    if (is_root(MPI_COMM_WORLD)) {
        auto request = async_send(std::span{values}, 1, MPI_COMM_WORLD);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    } else {
        MPI_Recv(values.data(), static_cast<int>(values.size()), MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }

    EXPECT_THAT(values, Eq(std::vector{1, 2, 3}));
}

TEST(AsyncSend, CanBeUsedWithNonFundamentalDatatypes) {
    auto values = is_root(MPI_COMM_WORLD)
                          ? std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                        AggregateData{"three", 3}}
                          : std::vector<AggregateData>{};

    if (is_root(MPI_COMM_WORLD)) {
        auto request = async_send(std::span{values}, 1, MPI_COMM_WORLD);
        MPI_Wait(&request, MPI_STATUS_IGNORE);
    } else {
        MPI_Status status;

        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

        int count;
        MPI_Get_count(&status, MPI_BYTE, &count);

        auto buffer = std::vector<std::byte>(count);

        MPI_Recv(buffer.data(), static_cast<int>(buffer.size()), MPI_BYTE, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        values = reshuffle::internal::deserialize<AggregateData>(buffer);
    }

    EXPECT_THAT(values, Eq(std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                       AggregateData{"three", 3}}));
}

TEST(BlockReceive, CanBeUsedToReceiveValuesFromOtherRank) {
    if (is_root(MPI_COMM_WORLD)) {
        const auto [source, values] = block_receive<int>(MPI_COMM_WORLD);

        EXPECT_THAT(source, Eq(1));
        EXPECT_THAT(values, Eq(std::vector{1, 2, 3}));
    } else {
        auto values = std::vector{1, 2, 3};
        MPI_Send(values.data(), static_cast<int>(values.size()), MPI_INT, 0, 0, MPI_COMM_WORLD);
    }
}

TEST(BlockReceive, CanBeUsedWithNonFundamentalDatatypes) {
    if (is_root(MPI_COMM_WORLD)) {
        const auto [source, values] = block_receive<AggregateData>(MPI_COMM_WORLD);

        EXPECT_THAT(source, Eq(1));
        EXPECT_THAT(values, Eq(std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                           AggregateData{"three", 3}}));
    } else {
        const auto values = std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                        AggregateData{"three", 3}};
        const auto bytes = reshuffle::internal::serialize(values);
        MPI_Send(bytes.data(), static_cast<int>(bytes.size()), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
    }
}

TEST(GetNumElements, ReturnsNumberOfElementsInAReceivedMessage) {
    auto values = is_root(MPI_COMM_WORLD) ? std::vector{1, 2, 3} : std::vector<int>(3);
    if (is_root(MPI_COMM_WORLD)) {
        MPI_Send(values.data(), static_cast<int>(values.size()), MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else {
        MPI_Status status{};
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        const auto count = get_num_elements(status, MPI_INT);
        MPI_Recv(values.data(), static_cast<int>(values.size()), MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        EXPECT_THAT(count, Eq(3));
    }
}
