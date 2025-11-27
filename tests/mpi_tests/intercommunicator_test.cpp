#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <intercommunicator.hpp>

#include <mpi_utils.hpp>

using namespace reshuffle::internal;
using namespace reshuffle::mpi;

using testing::Eq;

TEST(Intercommunicator, CreatesAnIntercommunicatorBetweenTwoCommunicators) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto intercomm = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);

    const auto num_ranks = get_num_ranks(MPI_COMM_WORLD);
    EXPECT_THAT(get_num_ranks(intercomm.get_intercommunicator()), Eq(num_ranks));
}

TEST(Intercommunicator, OnlyWorksIfOneCommunicatorIsSubcommunicatorOfTheOther) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    EXPECT_THROW(Intercommunicator(comm_rank_0, comm_rank_1), std::runtime_error);
}

TEST(Intercommunicator, CanGetRankInIntercommunicatorFromInitialCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    const auto intercomm_from_comm_rank_0 = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);
    const auto intercomm_from_comm_rank_1 = Intercommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(intercomm_from_comm_rank_0.get_intercomm_rank(
                        0, Intercommunicator::SelectCommunicator::INITIAL_COMM),
                Eq(0));
    EXPECT_THAT(intercomm_from_comm_rank_1.get_intercomm_rank(
                        0, Intercommunicator::SelectCommunicator::INITIAL_COMM),
                Eq(1));
}

TEST(Intercommunicator, CanGetRankInIntercommunicatorFromFinalCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    const auto intercomm_from_comm_rank_0 = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);
    const auto intercomm_from_comm_rank_1 = Intercommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(intercomm_from_comm_rank_0.get_intercomm_rank(
                        0, Intercommunicator::SelectCommunicator::FINAL_COMM),
                Eq(0));
    EXPECT_THAT(intercomm_from_comm_rank_1.get_intercomm_rank(
                        1, Intercommunicator::SelectCommunicator::FINAL_COMM),
                Eq(1));
}

TEST(Intercommunicator, ThrowsIfYouTryToGetRankIntercommunicatorFromARankNotInInitialCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    constexpr auto rank_not_in_first = 1;

    const auto intercomm = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THROW(auto _ = intercomm.get_intercomm_rank(
                         rank_not_in_first, Intercommunicator::SelectCommunicator::INITIAL_COMM),
                 std::invalid_argument);
}

TEST(Intercommunicator, ThrowsIfYouTryToGetRankIntercommunicatorFromARankNotInFinalCommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
    const auto rank_not_in_second = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD) + 1;

    const auto intercomm = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THROW(auto _ = intercomm.get_intercomm_rank(
                         rank_not_in_second, Intercommunicator::SelectCommunicator::FINAL_COMM),
                 std::invalid_argument);
}

TEST(Intercommunicator, CanGetRankInFinalCommunicatorFromRankIntercommunicator) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    const auto intercomm = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);

    EXPECT_THAT(intercomm.get_final_comm_rank(0), Eq(0));
    EXPECT_THAT(intercomm.get_final_comm_rank(1), Eq(1));
}

TEST(Intercommunicator, GetFinalCommRankUsesCallingRankIdByDefault) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    const auto intercomm = Intercommunicator(comm_rank_0, MPI_COMM_WORLD);

    if (const auto rank = get_rank_id(intercomm.get_intercommunicator()).value(); rank == 0) {
        EXPECT_THAT(intercomm.get_final_comm_rank(), Eq(0));
    } else {
        EXPECT_THAT(intercomm.get_final_comm_rank(), Eq(1));
    }
}

TEST(Intercommunicator, ReturnsNulloptWhileGettingRankIfIntercommRankNotInFinal) {
    const auto comm_rank_0 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));

    const auto intercomm = Intercommunicator(MPI_COMM_WORLD, comm_rank_0);

    EXPECT_THAT(intercomm.get_final_comm_rank(0), Eq(0));
    EXPECT_FALSE(intercomm.get_final_comm_rank(1).has_value());
}

TEST(Intercommunicator, CanGetRankInInitialCommunicatorFromRankIntercommunicator) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    const auto intercomm = Intercommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_THAT(intercomm.get_initial_comm_rank(1), Eq(0));
}

TEST(Intercommunicator, GetInitialCommRankUsesCallingRankIdByDefault) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    const auto intercomm = Intercommunicator(comm_rank_1, MPI_COMM_WORLD);

    if (const auto rank = get_rank_id(intercomm.get_intercommunicator()).value(); rank == 0) {
        EXPECT_FALSE(intercomm.get_initial_comm_rank().has_value());
    } else {
        EXPECT_THAT(intercomm.get_initial_comm_rank(), Eq(0));
    }
}

TEST(Intercommunicator, ReturnsNulloptWhileGettingRankIfIntercommRankNotInInitial) {
    const auto comm_rank_1 = get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));

    const auto intercomm = Intercommunicator(comm_rank_1, MPI_COMM_WORLD);

    EXPECT_FALSE(intercomm.get_initial_comm_rank(0).has_value());
}