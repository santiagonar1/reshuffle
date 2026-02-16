#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <processor_info.hpp>

using namespace heat;

TEST(AProcessorInfo, StoresInformationOfAProcessor) {
    const auto processor = ProcessorInfo(1, 2, 3, 4, 5);
    EXPECT_EQ(1, processor.get_rank());
    EXPECT_EQ(2, processor.get_up_neighbour());
    EXPECT_EQ(3, processor.get_down_neighbour());
    EXPECT_EQ(4, processor.get_left_neighbour());
    EXPECT_EQ(5, processor.get_right_neighbour());
}

TEST(AProcessorInfo, HasUpNeighborIfNotMPIProcNullPassed) {
    const auto processor_without_up_neighbor = ProcessorInfo(1, MPI_PROC_NULL, 2, 3, 4);
    EXPECT_FALSE(processor_without_up_neighbor.has_up_neighbour());

    const auto processor_with_up_neighbor =
            ProcessorInfo(1, 2, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL);
    EXPECT_TRUE(processor_with_up_neighbor.has_up_neighbour());
}

TEST(AProcessorInfo, HasDownNeighborIfNotMPIProcNullPassed) {
    const auto processor_without_down_neighbor = ProcessorInfo(1, 2, MPI_PROC_NULL, 3, 4);
    EXPECT_FALSE(processor_without_down_neighbor.has_down_neighbour());

    const auto processor_with_down_neighbor =
            ProcessorInfo(1, MPI_PROC_NULL, 3, MPI_PROC_NULL, MPI_PROC_NULL);
    EXPECT_TRUE(processor_with_down_neighbor.has_down_neighbour());
}

TEST(AProcessorInfo, HasLeftNeighborIfNotMPIProcNullPassed) {
    const auto processor_without_left_neighbor = ProcessorInfo(1, 2, 3, MPI_PROC_NULL, 4);
    EXPECT_FALSE(processor_without_left_neighbor.has_left_neighbour());

    const auto processor_with_left_neighbor =
            ProcessorInfo(1, MPI_PROC_NULL, MPI_PROC_NULL, 4, MPI_PROC_NULL);
    EXPECT_TRUE(processor_with_left_neighbor.has_left_neighbour());
}

TEST(AProcessorInfo, HasRightNeighborIfNotMPIProcNullPassed) {
    const auto processor_without_right_neighbor = ProcessorInfo(1, 2, 3, 4, MPI_PROC_NULL);
    EXPECT_FALSE(processor_without_right_neighbor.has_right_neighbour());

    const auto processor_with_right_neighbor =
            ProcessorInfo(1, MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL, 4);
    EXPECT_TRUE(processor_with_right_neighbor.has_right_neighbour());
}