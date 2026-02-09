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