#ifndef RESHUFFLE_BLOCK_HPP
#define RESHUFFLE_BLOCK_HPP

#include <span>
#include <vector>

#include "left_closed_range.hpp"
#include "rank_id.hpp"

#include <map>


namespace reshuffle::internal {
    class Block {
    public:
        Block();
        Block(const LeftClosedRange &interval, rank_id owner);

        [[nodiscard]] auto get_interval() const -> const LeftClosedRange &;
        [[nodiscard]] auto get_owner() const -> rank_id;
        [[nodiscard]] auto get_overlay(const Block &other) const -> std::optional<Block>;
        [[nodiscard]] auto get_num_elements() const -> int;

        [[nodiscard]] auto operator==(const Block &other) const -> bool;
        auto operator=(const Block &other) -> Block &;
        [[nodiscard]] auto operator<=>(const Block &other) const -> std::strong_ordering;


    private:
        LeftClosedRange _interval;
        rank_id _owner;
    };

    // Takes a group of disjoint blocks and transform them in a series of
    // contiguous blocks
    auto join(const std::vector<Block> &blocks) -> std::vector<Block>;

    auto get_num_elements_per_processor(const std::vector<Block> &blocks) -> std::map<rank_id, int>;

    auto group_by_processor(const std::vector<Block> &blocks) -> std::vector<Block>;

    template<typename T>
    auto extract_data(std::span<const T> data, const Block &block) -> std::span<const T> {
        const auto start = block.get_interval().get_left_bound();
        const auto finish = start + block.get_interval().get_length();

        if (start >= data.size() or finish > data.size()) {
            throw std::out_of_range("Block is out of bounds");
        }

        return {data.begin() + start, data.begin() + finish};
    }
}// namespace reshuffle::dev


#endif//RESHUFFLE_BLOCK_HPP
