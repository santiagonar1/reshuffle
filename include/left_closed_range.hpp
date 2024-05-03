#ifndef RESHUFFLE_LEFT_CLOSED_RANGE_HPP
#define RESHUFFLE_LEFT_CLOSED_RANGE_HPP

#include <utility>

namespace reshuffle::internal {
    using LeftClosedRange = std::pair<int, int>;

    bool in_range(const LeftClosedRange &range, int value) {
        return range.first <= value and value < range.second;
    }
}// namespace reshuffle::internal

#endif// RESHUFFLE_LEFT_CLOSED_RANGE_HPP
