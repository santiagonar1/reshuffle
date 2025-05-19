#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <multidimensional_data.hpp>

using namespace reshuffle::dev;

using testing::Eq;

TEST(Get1DData, Returns1DContainerOfMultidimensionalData) {
    const auto original_values = std::vector{1, 2, 3, 4, 5, 6};
    auto multidimensional_data = std::mdspan(original_values.data(), 2, 3);

    EXPECT_THAT(get_1D_data(multidimensional_data), Eq(original_values));
}


TEST(ExtractData, ReturnsTheDataContainedInMultidimensionalBlockRangeIn1D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{0, 1, 2, 3, 4, 5};
    auto multidimensional_data = std::mdspan(values.data(), 6);


    const auto block = MultidimensionalBlock{Block{{1, 3}, dummy_owner}};

    const auto expected = std::vector{1, 2};
    EXPECT_THAT(extract_data(multidimensional_data, block), Eq(expected));
}

TEST(ExtractData, ThrowsIfBlockIsOutOfBoundsIn1D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{0, 1, 2, 3, 4, 5};
    auto multidimensional_data = std::mdspan(values.data(), 6);


    const auto out_of_bounds_block =
            MultidimensionalBlock{Block{{1, static_cast<int>(values.size()) + 1}, dummy_owner}};

    EXPECT_THROW(auto _ = extract_data(multidimensional_data, out_of_bounds_block),
                 std::out_of_range);
}

TEST(ExtractData, ReturnsTheDataContainedInMultidimensionalBlockRangeIn2D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{1, 2, 3, 4, 5, 6};
    constexpr auto num_rows = 2;
    constexpr auto num_columns = 3;
    auto multidimensional_data = std::mdspan(values.data(), num_rows, num_columns);


    const auto block =
            MultidimensionalBlock{Block{{0, 2}, dummy_owner}, Block{{1, 3}, dummy_owner}};

    const auto expected = std::vector{2, 3, 5, 6};
    EXPECT_THAT(extract_data(multidimensional_data, block), Eq(expected));
}

TEST(ExtractData, ThrowsIfBlockIsOutOfBoundsIn2D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{0, 1, 2, 3, 4, 5};
    constexpr auto num_rows = 2;
    constexpr auto num_columns = 3;
    auto multidimensional_data = std::mdspan(values.data(), num_rows, num_columns);


    const auto out_of_bounds_block_rows = MultidimensionalBlock{
            Block{{0, num_rows + 1}, dummy_owner}, Block{{1, 3}, dummy_owner}};
    const auto out_of_bounds_block_columns = MultidimensionalBlock{
            Block{{0, 2}, dummy_owner}, Block{{1, num_columns + 1}, dummy_owner}};

    EXPECT_THROW(auto _ = extract_data(multidimensional_data, out_of_bounds_block_rows),
                 std::out_of_range);
    EXPECT_THROW(auto _ = extract_data(multidimensional_data, out_of_bounds_block_columns),
                 std::out_of_range);
}

TEST(CopyData, CopiesDataToMultidimensionalBlockRangeIn1D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{0, 1, 2, 3, 4, 5};

    const auto block = MultidimensionalBlock{Block{{2, 5}, dummy_owner}};
    auto destiny = std::vector<int>(values.size(), -1);


    auto destiny_mdspan = std::mdspan(destiny.data(), 6);

    copy_data(std::span{values}, destiny_mdspan, block);
    EXPECT_THAT(destiny, Eq(std::vector{-1, -1, 0, 1, 2, -1}));
}

TEST(CopyData, CopiesDataToMultidimensionalBlockRangeIn2D) {
    constexpr auto dummy_owner = -1;

    const auto values = std::vector{0, 1, 2, 3, 4, 5};

    // Copy the first two values into the first column of destiny
    const auto block =
            MultidimensionalBlock{Block{{0, 2}, dummy_owner}, Block{{0, 1}, dummy_owner}};
    auto destiny = std::vector<int>(values.size(), -1);

    constexpr auto num_rows = 2;
    constexpr auto num_columns = 3;
    auto destiny_mdspan = std::mdspan(destiny.data(), num_rows, num_columns);

    copy_data(std::span{values}, destiny_mdspan, block);
    EXPECT_THAT(destiny, Eq(std::vector{0, -1, -1, 1, -1, -1}));
}

TEST(GetNumElementsPerProcessor, ReturnsNumberOfElementsBasedOnMultidimensionalBlocks) {
    const auto processor_grid = ProcessorGrid<2>{{1, 2}};

    const auto multiblock_0 = MultidimensionalBlock{Block{{0, 2}, 0}, Block{{2, 4}, 0}};
    const auto multiblock_1 = MultidimensionalBlock{Block{{4, 6}, 0}, Block{{6, 8}, 1}};

    const auto blocks = std::vector{multiblock_0, multiblock_1, multiblock_0};
    const auto expected_num_elements_0 = 2 * get_num_elements(multiblock_0);
    const auto expected_num_elements_1 = get_num_elements(multiblock_1);

    const auto expected =
            std::map{std::pair{0, expected_num_elements_0}, std::pair{1, expected_num_elements_1}};
    EXPECT_THAT(get_num_elements_per_processor(blocks, processor_grid), Eq(expected));
}

TEST(GroupByProcessor, ReturnsA1DLayoutOfTheData) {
    const auto processor_grid = ProcessorGrid<2>{{1, 2}};

    const auto multiblock_0 = MultidimensionalBlock{Block{{0, 2}, 0}, Block{{2, 4}, 0}};
    const auto multiblock_1 = MultidimensionalBlock{Block{{4, 6}, 0}, Block{{6, 8}, 1}};

    const auto blocks = std::vector{multiblock_0, multiblock_1, multiblock_0};
    const auto expected_num_elements_0 = 2 * get_num_elements(multiblock_0);
    const auto expected_num_elements_1 = get_num_elements(multiblock_1);

    const auto expected_first_block = Block{{0, expected_num_elements_0}, 0};
    const auto expected_second_block =
            Block{{expected_num_elements_0, expected_num_elements_0 + expected_num_elements_1}, 1};

    EXPECT_THAT(group_by_processor(blocks, processor_grid),
                Eq(std::vector{expected_first_block, expected_second_block}));
}