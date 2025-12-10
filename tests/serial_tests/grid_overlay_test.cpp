#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_overlay.hpp>

using namespace reshuffle;
using namespace reshuffle::internal;

using testing::Eq;
using testing::UnorderedElementsAreArray;

TEST(GridOverlay, IsConstructedWithTwoGridLayouts) {
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 1}, 0}, Block{{1, 2}, 1}, Block{{2, 4}, 1}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = GridOverlay{origin_grid, target_grid};
}

TEST(GridOverlay, HasCoordinatesOwnersOriginGrid) {
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 4}, 0}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = GridOverlay{origin_grid, target_grid};

    const auto expected = std::vector{Coordinates{0}, Coordinates{1}};
    EXPECT_THAT(overlay.get_coordinates_owners_origin_grid(), Eq(expected));
}

TEST(GridOverlay, HasCoordinatesOwnersTargetGrid) {
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    const auto target_blocks = std::vector{{Block{{0, 4}, 0}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    const auto overlay = GridOverlay{origin_grid, target_grid};

    const auto expected = std::vector{Coordinates{0}, Coordinates{0}};
    EXPECT_THAT(overlay.get_coordinates_owners_target_grid(), Eq(expected));
}

TEST(GridOverlay, HasMultidimensionalBlocksOrigin) {
    // 0     2     4
    // |--0--|--1--|
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    // 0           4
    // |-----0-----|
    const auto target_blocks = std::vector{{Block{{0, 4}, 0}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    // 0     2     4
    // |-----|-----|
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // 0     2     4
    // |--0--|--1--|
    const auto expected = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}},
                                      MultidimensionalBlock{Block{{2, 4}, 1}}};

    EXPECT_THAT(overlay.get_multidimensional_blocks_origin(), Eq(expected));
}

TEST(GridOverlay, HasMultidimensionalBlocksTarget) {
    // 0     2     4
    // |--0--|--1--|
    const auto origin_blocks = std::vector{{Block{{0, 2}, 0}, Block{{2, 4}, 1}}};
    const auto origin_grid = GridLayout(std::array{origin_blocks});

    // 0           4
    // |-----0-----|
    const auto target_blocks = std::vector{{Block{{0, 4}, 0}}};
    const auto target_grid = GridLayout(std::array{target_blocks});

    // 0     2     4
    // |-----|-----|
    const auto overlay = GridOverlay{origin_grid, target_grid};

    // 0     2     4
    // |--0--|--0--|
    const auto expected = std::vector{MultidimensionalBlock{Block{{0, 2}, 0}},
                                      MultidimensionalBlock{Block{{2, 4}, 0}}};

    EXPECT_THAT(overlay.get_multidimensional_blocks_target(), Eq(expected));
}

TEST(GridOverlay, WorksIn2D) {
    //                 4        6
    //   +-------------+---------+
    //   |             |         |
    //   |             |         |
    //   |    (0,0)    |  (0,1)  |
    //   |             |         |
    //   |             |         |
    // 5 +-------------+---------+
    //   |    (1,0)    |  (1,1)  |
    // 6 +-------------+---------+
    const auto origin_blocks_x = std::vector{Block{{0, 4}, 0}, Block{{4, 6}, 1}};
    const auto origin_blocks_y = std::vector{Block{{0, 5}, 0}, Block{{5, 6}, 1}};
    const auto origin_grid = GridLayout(std::array{origin_blocks_y, origin_blocks_x});


    //           2              6
    //   +-------+---------------+
    //   |       |               |
    //   | (0,0) |     (0,1)     |
    //   |       |               |
    // 3 +-------+---------------+
    //   |       |               |
    //   | (1,0) |     (1,1)     |
    //   |       |               |
    // 6 +-------+---------------+
    const auto target_blocks_x = std::vector{Block{{0, 2}, 0}, Block{{2, 6}, 1}};
    const auto target_blocks_y = std::vector{Block{{0, 3}, 0}, Block{{3, 6}, 1}};
    const auto target_grid = GridLayout(std::array{target_blocks_y, target_blocks_x});

    //           2      4       6
    //   +-------+------+--------+
    //   |       |      |        |
    //   |       |      |        |
    //   |       |      |        |
    // 3 +-------+------+--------+
    //   |       |      |        |
    // 5 +-------+------+--------+
    //   |       |      |        |
    // 6 +-------+------+--------+
    const auto overlay = GridOverlay{origin_grid, target_grid};

    const auto expected_multidimensional_blocks_origin =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{3, 5}, 0}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{3, 5}, 0}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{2, 4}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{4, 6}, 1}}};

    EXPECT_THAT(overlay.get_multidimensional_blocks_origin(),
                UnorderedElementsAreArray(expected_multidimensional_blocks_origin));

    const auto expected_multidimensional_blocks_target =
            std::vector{MultidimensionalBlock{Block{{0, 3}, 0}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{2, 4}, 1}},
                        MultidimensionalBlock{Block{{0, 3}, 0}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{2, 4}, 1}},
                        MultidimensionalBlock{Block{{3, 5}, 1}, Block{{4, 6}, 1}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{0, 2}, 0}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{2, 4}, 1}},
                        MultidimensionalBlock{Block{{5, 6}, 1}, Block{{4, 6}, 1}}};

    EXPECT_THAT(overlay.get_multidimensional_blocks_target(),
                UnorderedElementsAreArray(expected_multidimensional_blocks_target));
}