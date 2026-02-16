#include "greedy_rank_order_strategy.hpp"

namespace reshuffle::internal {

    auto GreedyRankOrderStrategy::get_optimal_order(const Matrix2D<int> &matrix) const
            -> std::vector<RankId> {
        if (matrix.empty()) { return {}; }
        const auto rows = matrix.size();
        const auto cols = matrix[0].size();
        const auto num_considered_rows = std::min(rows, cols);
        auto permutation = std::vector<RankId>(cols, INVALID_RANK_ID);
        auto used = std::vector<bool>(cols, false);

        std::vector<int> row_sums(rows);
        for (std::size_t i = 0; i < rows; ++i) {
            row_sums[i] = std::accumulate(matrix[i].begin(), matrix[i].end(), 0);
        }

        for (std::size_t i = 0; i < num_considered_rows; ++i) {
            RankId best_rank = -1;
            int best_score = std::numeric_limits<int>::lowest();

            for (std::size_t candidate = 0; candidate < cols; ++candidate) {
                if (used[candidate]) continue;

                if (const int score = matrix[i][candidate] - row_sums[i]; score > best_score) {
                    best_score = score;
                    best_rank = static_cast<int>(candidate);
                }
            }

            permutation[i] = best_rank;
            used[best_rank] = true;
        }
        for (auto it = std::ranges::find(permutation, -1); it != permutation.end();
             it = std::ranges::find(permutation, -1)) {
            if (auto unused_it = std::ranges::find_if(used, [](const bool b) { return not b; });
                unused_it != used.end()) {
                const auto idx = static_cast<RankId>(std::distance(used.begin(), unused_it));
                *it = idx;
                *unused_it = true;
            }
        }

        if (cols < rows) {
            std::vector<RankId> extend_permutation(rows, INVALID_RANK_ID);
            std::ranges::copy(permutation, extend_permutation.begin());
            std::iota(extend_permutation.begin() + static_cast<long>(cols),
                      extend_permutation.end(), cols);
            permutation = extend_permutation;
        }
        return permutation;
    }
}// namespace reshuffle::internal
