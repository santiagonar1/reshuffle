#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <inter_communicator.hpp>

#include <mpi_utils.hpp>

using namespace reshuffle::internal;
using namespace reshuffle::mpi;

using testing::Eq;

TEST(InterCommunicator, CreatesAnIntercommunicatorBetweenTwoCommunicators) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto inter_comm = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);

    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    EXPECT_THAT(get_num_ranks(inter_comm.get_inter_communicator()), Eq(num_ranks));
}

TEST(InterCommunicator, OnlyWorksIfOneCommunicatorIsSubcommunicatorOfTheOther) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    EXPECT_THROW(InterCommunicator(comm_rank_0, comm_rank_1), std::runtime_error);
}

TEST(InterCommunicator, CanGetRankInIntercommunicatorFromInitialCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto inter_comm_from_comm_rank_0 = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);
    const auto inter_comm_from_comm_rank_1 = InterCommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(inter_comm_from_comm_rank_0.get_inter_comm_rank(
                        0, InterCommunicator::SelectCommunicator::INITIAL_COMM),
                Eq(0));
    EXPECT_THAT(inter_comm_from_comm_rank_1.get_inter_comm_rank(
                        0, InterCommunicator::SelectCommunicator::INITIAL_COMM),
                Eq(1));
}

TEST(InterCommunicator, CanGetRankInIntercommunicatorFromFinalCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto inter_comm_from_comm_rank_0 = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);
    const auto inter_comm_from_comm_rank_1 = InterCommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(inter_comm_from_comm_rank_0.get_inter_comm_rank(
                        0, InterCommunicator::SelectCommunicator::FINAL_COMM),
                Eq(0));
    EXPECT_THAT(inter_comm_from_comm_rank_1.get_inter_comm_rank(
                        1, InterCommunicator::SelectCommunicator::FINAL_COMM),
                Eq(1));
}

TEST(InterCommunicator, ThrowsIfYouTryToGetRankIntercommunicatorFromARankNotInInitialCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    constexpr auto rank_not_in_first = 1;

    const auto inter_comm = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THROW(auto _ = inter_comm.get_inter_comm_rank(
                         rank_not_in_first, InterCommunicator::SelectCommunicator::INITIAL_COMM),
                 std::invalid_argument);
}

TEST(InterCommunicator, ThrowsIfYouTryToGetRankIntercommunicatorFromARankNotInFinalCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});
    const auto rank_not_in_second = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD) + 1;

    const auto inter_comm = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THROW(auto _ = inter_comm.get_inter_comm_rank(
                         rank_not_in_second, InterCommunicator::SelectCommunicator::FINAL_COMM),
                 std::invalid_argument);
}

TEST(InterCommunicator, CanGetRankInFinalCommunicatorFromRankIntercommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});

    const auto inter_comm = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THAT(inter_comm.get_final_comm_rank(0), Eq(0));
    EXPECT_THAT(inter_comm.get_final_comm_rank(1), Eq(1));
}

TEST(InterCommunicator, GetFinalCommRankUsesCallingRankIdByDefault) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});

    const auto inter_comm = InterCommunicator(comm_rank_0, MPI_COMM_WORLD);

    if (const auto rank = get_rank_id(inter_comm.get_inter_communicator()).value(); rank == 0) {
        EXPECT_THAT(inter_comm.get_final_comm_rank(), Eq(0));
    } else {
        EXPECT_THAT(inter_comm.get_final_comm_rank(), Eq(1));
    }
}

TEST(InterCommunicator, ReturnsNulloptWhileGettingRankIfIntercommRankNotInFinal) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector{0});

    const auto inter_comm = InterCommunicator(MPI_COMM_WORLD, comm_rank_0);

    EXPECT_THAT(inter_comm.get_final_comm_rank(0), Eq(0));
    EXPECT_FALSE(inter_comm.get_final_comm_rank(1).has_value());
}

TEST(InterCommunicator, CanGetRankInInitialCommunicatorFromRankIntercommunicator) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto inter_comm = InterCommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(inter_comm.get_initial_comm_rank(1), Eq(0));
}

TEST(InterCommunicator, GetInitialCommRankUsesCallingRankIdByDefault) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto inter_comm = InterCommunicator(comm_rank_1, MPI_COMM_WORLD);

    if (const auto rank = get_rank_id(inter_comm.get_inter_communicator()).value(); rank == 0) {
        EXPECT_FALSE(inter_comm.get_initial_comm_rank().has_value());
    } else {
        EXPECT_THAT(inter_comm.get_initial_comm_rank(), Eq(0));
    }
}

TEST(InterCommunicator, ReturnsNulloptWhileGettingRankIfIntercommRankNotInInitial) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector{1});

    const auto inter_comm = InterCommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_FALSE(inter_comm.get_initial_comm_rank(0).has_value());
}