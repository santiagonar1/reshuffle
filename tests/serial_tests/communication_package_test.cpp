#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communication_package.hpp>

using namespace reshuffle::dev::internal;
using namespace reshuffle::dev;

using testing::Eq;


TEST(GetSendPackage, ConstructsACommunicationPackageBasedOnSendBlocks) {
    const auto data = std::vector{0, 1, 2, 3, 4, 5};
    const auto send_blocks = std::vector{Block{{0, 2}, 1}, Block{{2, 4}, 0}, Block{{4, 6}, 1}};

    const auto expected_data_send_to_0 = std::vector{2, 3};
    const auto expected_data_send_to_1 = std::vector{0, 1, 4, 5};
    const auto expected_destinies = std::vector{Block{{0, 2}, 0}, Block{{2, 6}, 1}};

    const auto num_values_for_0 = static_cast<int>(expected_data_send_to_0.size());

    auto expected_send_buffer = std::vector<int>(data.size());
    std::ranges::copy(expected_data_send_to_0, expected_send_buffer.begin());
    std::ranges::copy(expected_data_send_to_1, expected_send_buffer.begin() + num_values_for_0);

    const auto [send_buffer, destinies] = get_send_package(std::span{data}, send_blocks);

    EXPECT_THAT(send_buffer, Eq(expected_send_buffer));
    EXPECT_THAT(destinies, Eq(expected_destinies));
}