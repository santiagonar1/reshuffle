#include "general_data_distribution.hpp"

namespace reshuffle::distribution::internal {
    auto find_problematic_intervals(const std::vector<Interval> &intervals)
            -> std::optional<std::pair<Interval, Interval>> {
        if (intervals.empty() or intervals.size() == 1) { return std::nullopt; }

        const auto sorted_intervals = reshuffle::internal::sort(intervals);

        auto pairs =
                std::views::zip(std::views::drop(sorted_intervals, 1),
                                std::views::take(sorted_intervals, sorted_intervals.size() - 1));

        for (const auto &[one, other]: pairs) {
            if (not one.is_contiguous(other)) { return std::pair{one, other}; }
        }

        return std::nullopt;
    }
}// namespace reshuffle::distribution::internal