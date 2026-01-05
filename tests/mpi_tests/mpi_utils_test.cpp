#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi.h>
#include <mpi_utils.hpp>

#include "aggregate_data.hpp"
#include "autopas_particle.hpp"
#include "context_creation.hpp"
#include "fixed_size_data.hpp"

using namespace reshuffle::internal;
using namespace reshuffle::mpi;
using namespace reshuffle::mpi::internal;

using testing::Eq;

TEST(ToMPIDatatype, ConvertsDatatypeToMPIDatatype) {
    EXPECT_THAT(to_mpi_datatype<int>(), Eq(MPI_INT));
    EXPECT_THAT(to_mpi_datatype<float>(), Eq(MPI_FLOAT));
    EXPECT_THAT(to_mpi_datatype<double>(), Eq(MPI_DOUBLE));
    EXPECT_THAT(to_mpi_datatype<std::byte>(), Eq(MPI_BYTE));
    EXPECT_THAT(to_mpi_datatype<char>(), Eq(MPI_CHAR));
    EXPECT_THAT(to_mpi_datatype<unsigned char>(), Eq(MPI_UNSIGNED_CHAR));
    EXPECT_THAT(to_mpi_datatype<short>(), Eq(MPI_SHORT));
    EXPECT_THAT(to_mpi_datatype<unsigned short>(), Eq(MPI_UNSIGNED_SHORT));
    EXPECT_THAT(to_mpi_datatype<unsigned int>(), Eq(MPI_UNSIGNED));
    EXPECT_THAT(to_mpi_datatype<long>(), Eq(MPI_LONG));
    EXPECT_THAT(to_mpi_datatype<unsigned long>(), Eq(MPI_UNSIGNED_LONG));
    EXPECT_THAT(to_mpi_datatype<long long>(), Eq(MPI_LONG_LONG));
    EXPECT_THAT(to_mpi_datatype<unsigned long long>(), Eq(MPI_UNSIGNED_LONG_LONG));
    EXPECT_THAT(to_mpi_datatype<bool>(), Eq(MPI_C_BOOL));
    EXPECT_THAT(to_mpi_datatype<long double>(), Eq(MPI_LONG_DOUBLE));
}


TEST(GetRankId, ReturnsRankId) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    EXPECT_THAT(get_rank_id(MPI_COMM_WORLD).value(), Eq(rank));
}

TEST(GetRankId, ReturnsNullOptionIfCommIsNull) {
    EXPECT_FALSE(get_rank_id(MPI_COMM_NULL).has_value());
}

TEST(GetRankId, ReturnsNullOptionIfRankNotInComm) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});

    if (const auto rank = get_rank_id(MPI_COMM_WORLD).value(); rank == 0) {
        EXPECT_TRUE(get_rank_id(comm_rank_0).has_value());
    } else {
        EXPECT_FALSE(get_rank_id(comm_rank_0).has_value());
    }
}

TEST(IsRoot, ReturnsTrueForRankZero) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) { EXPECT_TRUE(is_root(MPI_COMM_WORLD)); }
}

TEST(IsRoot, ReturnsFalseIfRankNotZero) {
    auto rank{MPI_ERR_RANK};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank != 0) { EXPECT_FALSE(is_root(MPI_COMM_WORLD)); }
}

TEST(IsRoot, ReturnsFalseIfRankNotInComm) { EXPECT_FALSE(is_root(MPI_COMM_NULL)); }

TEST(GetNumRanks, ReturnsTheNumberOfRanks) {
    auto num_ranks{0};
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    EXPECT_THAT(get_num_ranks(MPI_COMM_WORLD), Eq(num_ranks));
}

TEST(GetNumRanks, ThrowsIfCommIsNull) {
    EXPECT_THROW(auto _ = get_num_ranks(MPI_COMM_NULL), std::invalid_argument);
}

TEST(IsCommNull, ReturnsTrueIfCommIsNull) { EXPECT_TRUE(is_comm_null(MPI_COMM_NULL)); }

TEST(IsCommNull, ReturnsFalseIfCommNotNull) { EXPECT_FALSE(is_comm_null(MPI_COMM_WORLD)); }

TEST(GetSubComm, CreatesAnMPISubcommunicator) {
    auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    if (is_root(MPI_COMM_WORLD)) {
        EXPECT_THAT(get_num_ranks(comm_rank_0), Eq(1));
        MPI_Comm_free(&comm_rank_0);
    }
}

TEST(BelongsToComm, ReturnsTrueIfRankInCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    if (is_root(MPI_COMM_WORLD)) { EXPECT_TRUE(reshuffle::mpi::belongs_to_comm(comm_rank_0)); }
}

TEST(BelongsToComm, ReturnsFalseIfRankNotInCommunicator) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    if (is_root(MPI_COMM_WORLD)) { EXPECT_FALSE(reshuffle::mpi::belongs_to_comm(comm_rank_1)); }
}

TEST(BelongsToComm, ReturnsFalseIfMPICommNullPassed) {
    EXPECT_FALSE(reshuffle::mpi::belongs_to_comm(MPI_COMM_NULL));
}

TEST(IsSubComm, ReturnsTrueIfSubComm) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    EXPECT_TRUE(reshuffle::mpi::is_sub_comm(MPI_COMM_WORLD, comm_rank_0));
}

TEST(IsSubComm, ReturnsFalseIfNotSubComm) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));
    EXPECT_FALSE(reshuffle::mpi::is_sub_comm(comm_rank_0, comm_rank_1));
}

TEST(IsSubComm, CommunicatorIsSubCommOfItself) {
    EXPECT_TRUE(reshuffle::mpi::is_sub_comm(MPI_COMM_WORLD, MPI_COMM_WORLD));
}


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

TEST(BlockScatter, CanScatterFundamentalDatatypes) {
    if (is_root(MPI_COMM_WORLD)) {
        auto values_to_scatter = std::vector{1, 2, 3};
        const auto values_per_rank = std::map<reshuffle::RankId, int>{{0, 1}, {1, 2}};
        const auto values =
                block_scatter(std::span{values_to_scatter}, values_per_rank, 0, MPI_COMM_WORLD);

        EXPECT_THAT(values, Eq(std::vector{1}));
    } else {
        auto dummy = std::vector<int>{};
        const auto values = block_scatter(std::span{dummy}, std::map<reshuffle::RankId, int>{}, 0,
                                          MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{2, 3}));
    }
}

TEST(BlockScatter, CanBeUsedWithNonFundamentalDatatypes) {
    if (is_root(MPI_COMM_WORLD)) {
        auto values_to_scatter = std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                             AggregateData{"three", 3}};
        const auto values_per_rank = std::map<reshuffle::RankId, int>{{0, 1}, {1, 2}};
        const auto values =
                block_scatter(std::span{values_to_scatter}, values_per_rank, 0, MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{AggregateData{"one", 1}}));
    } else {
        auto dummy = std::vector<AggregateData>{};
        const auto values = block_scatter(std::span{dummy}, std::map<reshuffle::RankId, int>{}, 0,
                                          MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{AggregateData{"two", 2}, AggregateData{"three", 3}}));
    }
}

TEST(BlockScatter, WorksWithFixedSizedDatatypes) {
    if (is_root(MPI_COMM_WORLD)) {
        auto values_to_scatter = std::vector{FixedSizeData{1}, FixedSizeData{2}, FixedSizeData{3}};
        const auto values_per_rank = std::map<reshuffle::RankId, int>{{0, 1}, {1, 2}};
        const auto values =
                block_scatter(std::span{values_to_scatter}, values_per_rank, 0, MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{FixedSizeData{1}}));
    } else {
        auto dummy = std::vector<FixedSizeData>{};
        const auto values = block_scatter(std::span{dummy}, std::map<reshuffle::RankId, int>{}, 0,
                                          MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{FixedSizeData{2}, FixedSizeData{3}}));
    }
}

TEST(BlockScatter, WorksWithAutopasParticles) {
    if (is_root(MPI_COMM_WORLD)) {
        auto values_to_scatter = std::vector<MoleculeLJ>(3);
        const auto values_per_rank = std::map<reshuffle::RankId, int>{{0, 1}, {1, 2}};
        const auto values =
                block_scatter(std::span{values_to_scatter}, values_per_rank, 0, MPI_COMM_WORLD);
        // EXPECT_THAT(values, Eq(std::vector{FixedSizeData{1}}));
    } else {
        auto dummy = std::vector<MoleculeLJ>{};
        const auto values = block_scatter(std::span{dummy}, std::map<reshuffle::RankId, int>{}, 0,
                                          MPI_COMM_WORLD);
        // EXPECT_THAT(values, Eq(std::vector{FixedSizeData{2}, FixedSizeData{3}}));
    }
}

TEST(BlockScatter, ValuesPerRankIsOnlyRelevantForRootRank) {
    if (is_root(MPI_COMM_WORLD)) {
        auto values_to_scatter = std::vector{AggregateData{"one", 1}, AggregateData{"two", 2},
                                             AggregateData{"three", 3}};
        const auto values_per_rank = std::map<reshuffle::RankId, int>{{0, 1}, {1, 2}};
        const auto values =
                block_scatter(std::span{values_to_scatter}, values_per_rank, 0, MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{AggregateData{"one", 1}}));
    } else {
        auto dummy = std::vector<AggregateData>{};
        const auto dummy_values_per_rank =
                std::map<reshuffle::RankId, int>{{0, 100'000}, {1, 100'000}};
        const auto values =
                block_scatter(std::span{dummy}, dummy_values_per_rank, 0, MPI_COMM_WORLD);
        EXPECT_THAT(values, Eq(std::vector{AggregateData{"two", 2}, AggregateData{"three", 3}}));
    }
}