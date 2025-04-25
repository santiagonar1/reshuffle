#ifndef CARTESIAN_VIEW_HPP
#define CARTESIAN_VIEW_HPP

#include <iterator>
#include <ranges>
#include <tuple>
#include <utility>

namespace reshuffle {
    namespace detail {
        template<typename Tuple, std::size_t... Is>
        auto tuple_refs_impl(Tuple &&tuple, std::index_sequence<Is...>) {
            return std::tuple<std::tuple_element_t<Is, std::remove_cvref_t<Tuple>> &...>(
                    std::get<Is>(std::forward<Tuple>(tuple))...);
        }

        template<typename Tuple>
        auto tuple_refs(Tuple &&tuple) {
            return tuple_refs_impl(
                    std::forward<Tuple>(tuple),
                    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>());
        }

        template<typename Tuple, std::size_t... Is>
        auto tuple_values_impl(Tuple &&tuple, std::index_sequence<Is...>) {
            return std::tuple<std::tuple_element_t<Is, std::remove_cvref_t<Tuple>>...>(
                    std::get<Is>(std::forward<Tuple>(tuple))...);
        }

        template<typename Tuple>
        auto tuple_values(Tuple &&tuple) {
            return tuple_values_impl(
                    std::forward<Tuple>(tuple),
                    std::make_index_sequence<std::tuple_size_v<std::remove_cvref_t<Tuple>>>());
        }

        // Helper to unpack array elements into a tuple
        template<typename T, std::size_t N, std::size_t... Is>
        auto array_to_tuple_impl(const std::array<std::vector<T>, N> &arr,
                                 std::index_sequence<Is...>) {
            // Create a tuple of N references to std::vector<T>
            return std::tuple(arr[Is]...);
        }

        template<typename T, std::size_t N>
        auto array_to_tuple(const std::array<std::vector<T>, N> &arr) {
            return array_to_tuple_impl(arr, std::make_index_sequence<N>{});
        }

        template<typename... Ranges>
        class cartesian_product_iterator {
        private:
            std::tuple<std::ranges::iterator_t<Ranges>...> current_iters_;
            std::tuple<std::ranges::sentinel_t<Ranges>...> end_iters_;
            bool is_end_ = false;
            std::tuple<std::remove_reference_t<Ranges> &...> ranges_;


            template<std::size_t I = 0>
            void increment() {
                if constexpr (I < sizeof...(Ranges)) {
                    auto &it = std::get<I>(current_iters_);
                    ++it;
                    if (it == std::get<I>(end_iters_)) {
                        if constexpr (I == 0) {
                            is_end_ = true;
                        } else {
                            it = std::ranges::begin(std::get<I>(ranges_));
                            increment<I - 1>();
                        }
                    }
                }
            }

            template<std::size_t I = sizeof...(Ranges) - 1>
            void init_end_check() {
                if constexpr (I < sizeof...(Ranges)) {
                    if (std::get<I>(current_iters_) == std::get<I>(end_iters_)) {
                        is_end_ = true;
                    } else {
                        init_end_check<I - 1>();
                    }
                }
            }

        public:
            using iterator_category = std::input_iterator_tag;
            using difference_type = std::ptrdiff_t;
            using value_type = std::tuple<std::ranges::range_value_t<Ranges>...>;
            using reference = std::tuple<std::ranges::range_reference_t<Ranges>...>;
            using pointer = void;

            cartesian_product_iterator() = delete;

            explicit cartesian_product_iterator(std::tuple<Ranges...> &ranges,
                                                const bool is_end = false)
                : ranges_(ranges), is_end_(is_end) {
                if (!is_end) {
                    std::apply(
                            [&](auto &...range) {
                                current_iters_ = std::make_tuple(std::ranges::begin(range)...);
                                end_iters_ = std::make_tuple(std::ranges::end(range)...);
                            },
                            ranges_);
                    init_end_check();
                }
            }


            reference operator*() const {
                return std::apply([](const auto &...iters) { return reference(*iters...); },
                                  current_iters_);
            }

            cartesian_product_iterator &operator++() {
                increment<sizeof...(Ranges) - 1>();
                return *this;
            }

            cartesian_product_iterator operator++(int) {
                auto tmp = *this;
                ++(*this);
                return tmp;
            }

            bool operator==(const cartesian_product_iterator &other) const {
                if (is_end_ && other.is_end_) return true;
                if (is_end_ != other.is_end_) return false;
                return current_iters_ == other.current_iters_;
            }

            bool operator!=(const cartesian_product_iterator &other) const {
                return !(*this == other);
            }
        };
    }// namespace detail

    template<std::ranges::input_range... Ranges>
        requires(std::ranges::forward_range<Ranges> && ...)
    class cartesian_product_view
        : public std::ranges::view_interface<cartesian_product_view<Ranges...>> {
    private:
        std::tuple<Ranges...> ranges_;

    public:
        cartesian_product_view() = default;
        explicit cartesian_product_view(Ranges... ranges) : ranges_(std::move(ranges)...) {}

        cartesian_product_view(cartesian_product_view &&) = default;
        cartesian_product_view &operator=(cartesian_product_view &&) = default;

        auto begin() { return detail::cartesian_product_iterator<Ranges...>(ranges_); }

        auto begin() const { return detail::cartesian_product_iterator<const Ranges...>(ranges_); }

        auto end() { return detail::cartesian_product_iterator<Ranges...>(ranges_, true); }

        auto end() const {
            return detail::cartesian_product_iterator<const Ranges...>(ranges_, true);
        }

        auto empty() const {
            return std::apply(
                    [](const auto &...ranges) { return ((std::ranges::empty(ranges)) || ...); },
                    ranges_);
        }
    };

    template<typename... Rs>
    cartesian_product_view(Rs &&...) -> cartesian_product_view<std::views::all_t<Rs>...>;

    namespace views {
        struct cartesian_product_fn {
            template<std::ranges::input_range... Ranges>
                requires(std::ranges::forward_range<Ranges> && ...)
            auto operator()(Ranges &&...ranges) const {
                return cartesian_product_view<std::views::all_t<Ranges>...>(
                        std::views::all(std::forward<Ranges>(ranges))...);
            }

            // Overload for array of vectors - unpack and forward to variadic version
            template<typename T, std::size_t N>
            auto operator()(const std::array<std::vector<T>, N> &array) const {
                auto tuple = detail::array_to_tuple(array);
                return std::apply(cartesian_product_fn{}, detail::array_to_tuple(array));
            }
        };

        inline constexpr cartesian_product_fn cartesian_product{};
    }// namespace views
}// namespace reshuffle

// Enable view concept
template<typename... Ranges>
inline constexpr bool std::ranges::enable_view<reshuffle::cartesian_product_view<Ranges...>> = true;


#endif//CARTESIAN_VIEW_HPP
