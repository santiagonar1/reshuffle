#include "left_closed_range.hpp"

#include <utility>

namespace reshuffle::internal {
    LeftClosedRange::LeftClosedRange() : _interval(){};
    LeftClosedRange::LeftClosedRange(int left_bound, int right_bound)
        : _interval{left_bound, right_bound} {};

    auto LeftClosedRange::contains(int value) const -> bool {
        return _interval.first <= value and value < _interval.second;
    }

    auto LeftClosedRange::get_left_bound() const -> int { return _interval.first; }
    auto LeftClosedRange::get_right_bound() const -> int { return _interval.second; }
    auto LeftClosedRange::get_length() const -> int { return _interval.second - _interval.first; }

    auto LeftClosedRange::operator==(const LeftClosedRange &other) const -> bool {
        return _interval == other._interval;
    }
}// namespace reshuffle::internal
