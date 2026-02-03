#ifndef RESHUFFLE_MULTIDIMENSIONAL_INTERVAL_HPP
#define RESHUFFLE_MULTIDIMENSIONAL_INTERVAL_HPP

#include "interval.hpp"
#include "left_closed_range.hpp"
#include "utils.hpp"

#include <array>
#include <vector>

namespace reshuffle {
    template<std::size_t N>
    using MultidimensionalInterval = std::array<Interval, N>;

    namespace internal {
        template<std::size_t N>
        [[nodiscard]] auto to_unidimensional_intervals(const MultidimensionalInterval<N> &interval)
                -> std::vector<Interval> {
            auto unidimensional_intervals = std::vector<Interval>{};
            for (int dim = 0; dim < N; ++dim) {
                unidimensional_intervals.emplace_back(interval[dim]);
            }
            return unidimensional_intervals;
        }


        template<std::size_t N>
        [[nodiscard]] auto
        to_unidimensional_intervals(const std::vector<MultidimensionalInterval<N>> &intervals)
                -> std::array<std::vector<Interval>, N> {

            auto unidimensional_intervals = std::array<std::vector<Interval>, N>{};

            for (const auto &multidimensional_interval: intervals) {
                for (int dim = 0; dim < N; ++dim) {
                    unidimensional_intervals[dim].emplace_back(multidimensional_interval[dim]);
                }
            }

            for (auto &unidimensional_interval: unidimensional_intervals) {
                unidimensional_interval = remove_duplicates(unidimensional_interval);
            }

            return unidimensional_intervals;
        }

    }// namespace internal
}// namespace reshuffle

#endif//RESHUFFLE_MULTIDIMENSIONAL_INTERVAL_HPP
