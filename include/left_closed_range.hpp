#ifndef RESHUFFLE_LEFT_CLOSED_RANGE_HPP
#define RESHUFFLE_LEFT_CLOSED_RANGE_HPP

#include <utility>

namespace reshuffle::internal {
    class LeftClosedRange {
    private:
        const std::pair<int, int> _interval;

    public:
        LeftClosedRange() : _interval(){};
        LeftClosedRange(int left_bound, int right_bound) : _interval{left_bound, right_bound} {};

        [[nodiscard]] bool contains(int value) const {
            return _interval.first <= value and value < _interval.second;
        }

        [[nodiscard]] int get_left_bound() const { return _interval.first; }
        [[nodiscard]] int get_right_bound() const { return _interval.second; }
        [[nodiscard]] int get_length() const { return _interval.second - _interval.first; }

        bool operator==(const LeftClosedRange &other) const { return _interval == other._interval; }
    };
}// namespace reshuffle::internal

#endif// RESHUFFLE_LEFT_CLOSED_RANGE_HPP
