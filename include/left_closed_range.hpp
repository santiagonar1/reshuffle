#ifndef RESHUFFLE_LEFT_CLOSED_RANGE_HPP
#define RESHUFFLE_LEFT_CLOSED_RANGE_HPP

#include <utility>

namespace reshuffle::internal {
    class LeftClosedRange {
    public:
        LeftClosedRange(int left_bound, int right_bound);

        [[nodiscard]] auto contains(int value) const -> bool;
        [[nodiscard]] auto get_left_bound() const -> int;
        [[nodiscard]] auto get_right_bound() const -> int;
        [[nodiscard]] auto get_length() const -> int;

        auto operator==(const LeftClosedRange &other) const -> bool;

    private:
        const std::pair<int, int> _interval;
    };
}// namespace reshuffle::internal

#endif// RESHUFFLE_LEFT_CLOSED_RANGE_HPP
