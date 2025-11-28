#ifndef RESHUFFLE_DIMENSIONS_HPP
#define RESHUFFLE_DIMENSIONS_HPP

#include <algorithm>
#include <array>
#include <numeric>

#include "mdspan.hpp"
#include "profiler.hpp"

namespace reshuffle {
    template<std::size_t N>
    using Dimensions = std::array<int, N>;

    template<std::size_t N>
    auto calc_total_num_values(const Dimensions<N> &d) -> int {
        return std::accumulate(d.cbegin(), d.cend(), 1, std::multiplies());
    }

    namespace internal {
        template<typename C>
        constexpr auto get_rank_impl() -> int {
            if constexpr (requires { typename C::value_type; }) {
                using ElementType = typename C::value_type;
                if constexpr (
                        requires { typename ElementType::value_type; } &&
                        requires(const ElementType &e) { e.size(); }) {
                    // If the elements are containers themselves, recursively get their rank
                    return 1 + get_rank_impl<ElementType>();
                }
            }
            // Base case: the elements are not containers
            return 1;
        }

        // Convenience overload that deduces the container type from an instance
        template<typename C>
        constexpr auto get_rank(const C &) -> int {
            return get_rank_impl<C>();
        }

        template<typename T, typename Extents>
        auto get_dimensions(std::mdspan<const T, Extents> local_values)
                -> Dimensions<Extents::rank()> {
            PROFILE_SCOPE_NAMED("mdspan::get_dimensions");
            constexpr auto N = Extents::rank();

            if (local_values.empty()) { return Dimensions<N>{}; }

            auto dimensions = Dimensions<N>{};
            for (int i = 0; i < N; ++i) { dimensions[i] = local_values.extent(i); }

            return dimensions;
        }

        template<typename C>
        constexpr auto get_dimensions(const C &container) -> Dimensions<get_rank_impl<C>()> {
            PROFILE_SCOPE_NAMED("container::get_dimensions");
            auto dimensions = Dimensions<get_rank_impl<C>()>{};

            if (container.empty()) { return dimensions; }

            // Add size of the current dimension
            dimensions[0] = container.size();

            // Check if we have nested containers
            if constexpr (requires { typename C::value_type; }) {
                using ElementType = typename C::value_type;
                if constexpr (requires(const ElementType &e) { e.size(); }) {
                    if (!container.empty()) {
                        // Get dimensions of nested containers
                        auto nested_dimensions = get_dimensions(*container.begin());
                        std::copy(nested_dimensions.begin(), nested_dimensions.end(),
                                  dimensions.begin() + 1);

                        // Update with maximum sizes from other elements
                        for (auto it = std::next(container.begin()); it != container.end(); ++it) {
                            auto current_dims = get_dimensions(*it);
                            for (size_t i = 0; i < current_dims.size(); ++i) {
                                dimensions[i + 1] = std::max(dimensions[i + 1], current_dims[i]);
                            }
                        }
                    }
                    // If the container is empty, the remaining dimensions are already 0 from initialization
                }
            }

            return dimensions;
        }
    }// namespace internal
}// namespace reshuffle

#endif//RESHUFFLE_DIMENSIONS_HPP
