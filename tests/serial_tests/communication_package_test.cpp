#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communication_package.hpp>

using namespace reshuffle::internal;
using namespace reshuffle;

using testing::Eq;


TEST(GetSendPackage, ConstructsACommunicationPackageBasedOnSendBlocksWithMultidimensionalBlocks) {
    const auto data = std::vector{0, 1, 2, 3, 4, 5};
    const auto send_blocks = std::vector{MultidimensionalBlock<1>{Block{{0, 2}, 1}},
                                         MultidimensionalBlock<1>{Block{{2, 4}, 0}},
                                         MultidimensionalBlock<1>{Block{{4, 6}, 1}}};
    const auto processor_grid = ProcessorGrid<1>{{2}};

    const auto expected_data_send_to_0 = std::vector{2, 3};
    const auto expected_data_send_to_1 = std::vector{0, 1, 4, 5};
    const auto expected_destinies = std::map<RankId, LeftClosedRange>{{0, {0, 2}}, {1, {2, 6}}};

    const auto num_values_for_0 = static_cast<int>(expected_data_send_to_0.size());

    auto expected_send_buffer = std::vector<int>(data.size());
    std::ranges::copy(expected_data_send_to_0, expected_send_buffer.begin());
    std::ranges::copy(expected_data_send_to_1, expected_send_buffer.begin() + num_values_for_0);

    const auto [send_buffer, destinies] =
            get_send_package(std::mdspan{data.data(), 6}, send_blocks, processor_grid);

    EXPECT_THAT(send_buffer, Eq(expected_send_buffer));
    EXPECT_THAT(destinies, Eq(expected_destinies));
}

TEST(GetSendPackage, WorksIn2D) {
    constexpr auto num_rows = 3;
    constexpr auto num_columns = 3;
    const auto data = std::vector{0, 1, 2, 3, 4, 5, 6, 7, 8};
    const auto send_blocks =
            std::vector{MultidimensionalBlock<2>{Block{{0, 2}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock<2>{Block{{0, 2}, 0}, Block{{2, 3}, 1}},
                        MultidimensionalBlock<2>{Block{{2, 3}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock<2>{Block{{2, 3}, 1}, Block{{2, 3}, 1}}};
    const auto processor_grid = ProcessorGrid<2>{{2, 2}};

    const auto expected_destinies =
            std::map<RankId, LeftClosedRange>{{0, {0, 4}}, {1, {4, 6}}, {2, {6, 8}}, {3, {8, 9}}};

    const auto expected_send_buffer = std::vector{0, 1, 3, 4, 2, 5, 6, 7, 8};

    const auto [send_buffer, destinies] = get_send_package(
            std::mdspan{data.data(), num_rows, num_columns}, send_blocks, processor_grid);

    EXPECT_THAT(send_buffer, Eq(expected_send_buffer));
    EXPECT_THAT(destinies, Eq(expected_destinies));
}