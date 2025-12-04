#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <grid_overlay.hpp>

using namespace reshuffle::internal;

using testing::Eq;

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