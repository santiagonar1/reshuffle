#ifndef RESHUFFLE_COLORING_HPP
#define RESHUFFLE_COLORING_HPP

#include <functional>
#include <numeric>
#include <algorithm>
#include "utils.hpp"
#include "dimensions.hpp"
#include "left_closed_range.hpp"

namespace reshuffle {
    using ColoringDescriptor = std::vector<internal::LeftClosedRange>;

    namespace internal {
        int get_color(const ColoringDescriptor &coloring_descriptor, int i) {
            auto it = std::ranges::find_if(coloring_descriptor, [i](const auto &r) { return in_range(r, i); });
            //TODO: Should we check whether the index requested is out of bounds?
            return static_cast<int>(std::distance(coloring_descriptor.begin(), it));
        }

        std::pair<int, int> to_2D(int num_columns, int index) {
            return {index % num_columns, index / num_columns};
        }

    }

    class BlockWise {
    private:
        const int _num_blocks;

        [[nodiscard]] int get_min_values_per_block(int num_values) const {
            return num_values / _num_blocks;
        }

    public:
        explicit BlockWise(int num_blocks) : _num_blocks(num_blocks) {}

        [[nodiscard]] ColoringDescriptor get_coloring_descriptor(int num_values) const {
            const auto min_values_per_block = get_min_values_per_block(num_values);
            ColoringDescriptor coloring_descriptor{};

            for (int i = 0; i < min_values_per_block * _num_blocks; i += min_values_per_block) {
                const auto starting_index = i;
                const auto last_index = starting_index + min_values_per_block;
                coloring_descriptor.emplace_back(starting_index, last_index);
            }

            coloring_descriptor.back().second = num_values;
            return coloring_descriptor;
        }
    };

    std::vector<int> create_coloring(const std::vector<int> &global_coloring,
                                     const BlockWise &strategy, int rank) {
        const auto num_values = static_cast<int>(global_coloring.size());
        const auto coloring_descriptor = strategy.get_coloring_descriptor(num_values);

        auto coloring = std::vector<int>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            if (rank == global_coloring[i]) {
                coloring.push_back(internal::get_color(coloring_descriptor, i));
            }
        }

        return coloring;
    }

    std::vector<int> create_coloring(const std::vector<int> &global_coloring,
                                     const Dimensions2D &global_dimensions,
                                     const std::array<BlockWise, 2> &strategies, int rank) {
        const auto coloring_x = strategies[0].get_coloring_descriptor(global_dimensions.num_columns);
        const auto coloring_y = strategies[1].get_coloring_descriptor(global_dimensions.num_rows);
        const auto combination = internal::combine(coloring_x, coloring_y);

        auto coloring = std::vector<int>{};
        for (int i = 0; i < global_coloring.size(); ++i) {
            if (rank == global_coloring[i]) {
                const auto [x_coord, y_coord] = internal::to_2D(global_dimensions.num_columns, i);

                auto it = std::ranges::find_if(combination, [x_coord, y_coord](const auto &r) {
                    return internal::in_range(r.first, x_coord) and internal::in_range(r.second, y_coord);
                });
                coloring.push_back(static_cast<int>(std::distance(combination.begin(), it)));
            }
        }

        return coloring;
    }
}

#endif //RESHUFFLE_COLORING_HPP
