#ifndef RESHUFFLE_DIMENSIONS_HPP
#define RESHUFFLE_DIMENSIONS_HPP

#include <array>
#include <cassert>
#include <numeric>

namespace reshuffle {
    namespace internal {
        template<std::size_t N>
        std::array<int, N> to_array(const std::initializer_list<int> &list) {
            assert(list.size() == N);
            std::array<int, N> arr;
            std::copy(list.begin(), list.end(), arr.begin());
            return arr;
        }
    }// namespace internal

    struct Dimensions2D {
        const int num_rows{};
        const int num_columns{};

        [[nodiscard]] auto get_num_values() const { return num_rows * num_columns; }
    };

    template<std::size_t N>
    class Dimension {
    private:
        const std::array<int, N> _num_values_per_dimension;

    public:
        Dimension(const std::initializer_list<int> &num_values_per_dimension)
            : _num_values_per_dimension(internal::to_array<N>(num_values_per_dimension)) {
            assert(num_values_per_dimension.size() == N);
        }

        [[nodiscard]] auto get_num_values_dim(int dimension) const -> int {
            return _num_values_per_dimension.at(dimension);
        }

        [[nodiscard]] auto get_total_number_of_values() const -> int {
            return std::accumulate(_num_values_per_dimension.cbegin(),
                                   _num_values_per_dimension.cend(), 1, std::multiplies());
        }
    };
}// namespace reshuffle

#endif//RESHUFFLE_DIMENSIONS_HPP
