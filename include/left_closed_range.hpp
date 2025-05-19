#ifndef RESHUFFLE_LEFT_CLOSED_RANGE_HPP
#define RESHUFFLE_LEFT_CLOSED_RANGE_HPP

#include <iterator>
#include <optional>
#include <utility>

namespace reshuffle::internal {
    class LeftClosedRange {
    public:
        class iterator {
        public:
            using iterator_category = std::forward_iterator_tag;
            using value_type = int;
            using difference_type = std::ptrdiff_t;
            using pointer = const int *;
            using reference = const int &;

            explicit iterator(int value) : current(value) {}

            auto operator*() const -> reference { return current; }
            auto operator->() const -> pointer { return &current; }

            auto operator++() -> iterator & {
                ++current;
                return *this;
            }

            auto operator++(int) -> iterator {
                iterator tmp = *this;
                ++current;
                return tmp;
            }

            auto operator==(const iterator &other) const -> bool {
                return current == other.current;
            }

        private:
            int current;
        };

        LeftClosedRange(int left_bound, int right_bound);
        LeftClosedRange();

        [[nodiscard]] auto begin() const -> iterator;
        [[nodiscard]] auto end() const -> iterator;


        [[nodiscard]] auto contains(int value) const -> bool;
        [[nodiscard]] auto get_left_bound() const -> int;
        [[nodiscard]] auto get_right_bound() const -> int;
        [[nodiscard]] auto get_length() const -> int;
        [[nodiscard]] auto get_overlay(const LeftClosedRange &other) const
                -> std::optional<LeftClosedRange>;

        auto operator==(const LeftClosedRange &other) const -> bool;
        auto operator=(const LeftClosedRange &other) -> LeftClosedRange &;
        [[nodiscard]] auto operator<=>(const LeftClosedRange &other) const -> std::strong_ordering;

    private:
        std::pair<int, int> _interval;
    };
}// namespace reshuffle::internal

#endif// RESHUFFLE_LEFT_CLOSED_RANGE_HPP
