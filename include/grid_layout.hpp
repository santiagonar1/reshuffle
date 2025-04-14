#ifndef GRID_LAYOUT_HPP
#define GRID_LAYOUT_HPP

#include "block.hpp"
#include "rank_id.hpp"

#include <span>
#include <vector>

namespace reshuffle::dev {

    struct GridOverlay;

    class GridLayout {
    public:
        explicit GridLayout(std::vector<Block> blocks);

        [[nodiscard]] auto get_blocks() const -> const std::vector<Block> &;
        [[nodiscard]] auto get_block_owner(int block_id) const -> rank_id;
        [[nodiscard]] auto get_overlay(const GridLayout &target_grid) const -> GridOverlay;
        [[nodiscard]] auto get_local_grid(rank_id rank) const -> GridLayout;

    private:
        const std::vector<Block> _blocks;
    };

    struct GridOverlay {
        GridLayout grid;
        std::vector<rank_id> owners_target_grid;
    };
}// namespace reshuffle::dev

#endif//GRID_LAYOUT_HPP
