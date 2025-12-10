#include "left_closed_range.hpp"

#include <algorithm>
#include <ostream>
#include <ranges>
#include <stdexcept>
#include <utility>

namespace reshuffle {
    LeftClosedRange::LeftClosedRange(int left_bound, int right_bound)
        : _interval{left_bound, right_bound} {
        if (left_bound > right_bound) {
            throw std::invalid_argument("The right bound cannot be smaller than the left bound");
        }
    }

    LeftClosedRange::LeftClosedRange() : LeftClosedRange(0, 0) {}

    auto LeftClosedRange::begin() const -> iterator { return iterator(_interval.first); }
    auto LeftClosedRange::end() const -> iterator { return iterator(_interval.second); }


    auto LeftClosedRange::contains(const int value) const -> bool {
        return _interval.first <= value and value < _interval.second;
    }

    auto LeftClosedRange::get_left_bound() const -> int { return _interval.first; }
    auto LeftClosedRange::get_right_bound() const -> int { return _interval.second; }
    auto LeftClosedRange::get_length() const -> int { return _interval.second - _interval.first; }

    auto LeftClosedRange::get_overlay(const LeftClosedRange &other) const
            -> std::optional<LeftClosedRange> {
        if (other.get_left_bound() >= get_right_bound() or
            other.get_right_bound() <= get_left_bound()) {
            return std::nullopt;
        }

        const auto left_bound = std::max(get_left_bound(), other.get_left_bound());
        const auto right_bound = std::min(get_right_bound(), other.get_right_bound());
        return LeftClosedRange{left_bound, right_bound};
    }

    auto LeftClosedRange::operator==(const LeftClosedRange &other) const -> bool {
        return _interval == other._interval;
    }

    auto LeftClosedRange::operator=(const LeftClosedRange &other) -> LeftClosedRange & = default;

    auto LeftClosedRange::operator<=>(const LeftClosedRange &other) const -> std::strong_ordering {
        return _interval.second <=> other._interval.second;
    }

    auto chain(const std::vector<LeftClosedRange> &intervals) -> std::vector<LeftClosedRange> {
        if (intervals.empty()) { return {}; }

        auto chained_intervals = std::vector{LeftClosedRange{0, intervals.front().get_length()}};
        for (const auto &interval: intervals | std::views::drop(1)) {
            const auto last_interval = chained_intervals.back();
            const auto from = last_interval.get_right_bound();
            const auto to = from + interval.get_length();
            chained_intervals.emplace_back(from, to);
        }

        return chained_intervals;
    }

    auto operator<<(std::ostream &os, const LeftClosedRange &range) -> std::ostream & {
        return os << "[" << range.get_left_bound() << ", " << range.get_right_bound() << ")";
    }

}// namespace reshuffle
