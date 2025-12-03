#ifndef RESHUFFLE_BLOCK_OVERLAY_HPP
#define RESHUFFLE_BLOCK_OVERLAY_HPP

#include "block.hpp"
#include "left_closed_range.hpp"
#include "rank_id.hpp"

#include <ranges>

namespace reshuffle::internal {
    struct BlockOverlay {
        LeftClosedRange interval;
        RankId id_origin;
        RankId id_target;

        auto operator==(const BlockOverlay &) const -> bool = default;
    };

    auto operator<<(std::ostream &os, const BlockOverlay &block_overlay) -> std::ostream &;

    [[nodiscard]] auto get_blocks_overlay(const Block &origin, const Block &target)
            -> std::optional<BlockOverlay>;

    [[nodiscard]] auto get_blocks_overlay(const std::vector<Block> &origin_blocks,
                                          const std::vector<Block> &target_blocks)
            -> std::vector<BlockOverlay>;

    template<std::size_t N>
    [[nodiscard]] auto get_blocks_overlay(const std::array<std::vector<Block>, N> &origin_blocks,
                                          const std::array<std::vector<Block>, N> &target_blocks)
            -> std::array<std::vector<BlockOverlay>, N>;

    template<std::size_t N>
    auto get_blocks_overlay(const std::array<std::vector<Block>, N> &origin_blocks,
                            const std::array<std::vector<Block>, N> &target_blocks)
            -> std::array<std::vector<BlockOverlay>, N> {

        auto result = std::array<std::vector<BlockOverlay>, N>{};

        int i{};
        for (const auto &[origin, target]: std::views::zip(origin_blocks, target_blocks)) {
            result[i] = get_blocks_overlay(origin, target);
            i++;
        }

        return result;
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_BLOCK_OVERLAY_HPP
