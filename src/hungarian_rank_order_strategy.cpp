#include "hungarian_rank_order_strategy.hpp"

namespace reshuffle::internal {
    auto HungarianRankOrderStrategy::get_optimal_order(const Matrix2D<int> &matrix) const
            -> std::vector<rank_id> {
        if (matrix.empty()) { return {}; }

        const auto rows = matrix.size();
        const auto cols = matrix[0].size();
        auto n = std::max(rows, cols);

        // generation of cost matrix since we want to transform our maximization problem into a
        // minimization problem, which can be handled with the Hungarian algorythm
        int max_val = 0;
        for (const auto &row: matrix) {
            for (int w: row) {
                if (w > max_val) max_val = w;
            }
        }
        const int dummy_weight = max_val + 1;
        std::vector<std::vector<int>> a(n + 1, std::vector<int>(n + 1, dummy_weight));
        for (auto i = 1; i <= rows; ++i) {
            for (auto j = 1; j <= cols; ++j) {
                const int weight = matrix[i - 1][j - 1];
                a[i][j] = max_val - weight;
            }
        }

        // we are interested in target rank reordering, so the number of cols is governing
        n = cols;

        // hungarian algorithm (Kuhn - Munkres) with index starting at 1
        constexpr int high_weight = std::numeric_limits<int>::max() / 4;
        std::vector<int> u(n + 1, 0), v(n + 1, 0);
        std::vector<int> p(n + 1, 0), way(n + 1, 0);

        for (auto i = 1; i <= n; ++i) {
            p[0] = i;
            int j0 = 0;
            std::vector<int> minv(n + 1, high_weight);
            std::vector<bool> used(n + 1, false);

            do {
                used[j0] = true;
                const int i0 = p[j0];
                int delta = high_weight, j1 = 0;
                for (auto j = 1; j <= n; ++j) {
                    if (used[j]) continue;
                    const int cur = a[i0][j] - u[i0] - v[j];
                    if (cur < minv[j]) {
                        minv[j] = cur;
                        way[j] = j0;
                    }
                    if (minv[j] < delta) {
                        delta = minv[j];
                        j1 = j;
                    }
                }
                for (auto j = 0; j <= n; ++j) {
                    if (used[j]) {
                        u[p[j]] += delta;
                        v[j] -= delta;
                    } else {
                        minv[j] -= delta;
                    }
                }
                j0 = j1;
            } while (p[j0] != 0);
            do {
                const int j1 = way[j0];
                p[j0] = p[j1];
                j0 = j1;
            } while (j0 != 0);
        }

        // p now contains the mapping by the following rule:
        // permutation[p[j]] = j
        std::vector<rank_id> permutation(cols, -1);
        std::vector<bool> col_used(cols, false);

        for (auto j = 1; j <= n; ++j) {
            const int i = p[j];
            const bool real_row = (i >= 1) && (static_cast<std::size_t>(i) <= rows);
            const bool real_col = static_cast<std::size_t>(j) <= cols;
            if (real_row && real_col) {
                const auto row_idx = static_cast<std::size_t>(i - 1);
                const auto col_idx = static_cast<std::size_t>(j - 1);
                if (row_idx < rows && col_idx < cols && row_idx < cols) {
                    permutation[row_idx] = static_cast<rank_id>(col_idx);
                    col_used[col_idx] = true;
                }
            }
        }

        // filling up unused cols in a stable manner
        auto fill_pos = std::min(rows, cols);
        for (auto c = 0; c < cols; ++c) {
            if (!col_used[c]) {
                while (fill_pos < cols && permutation[fill_pos] != -1) { ++fill_pos; }
                if (fill_pos < cols) {
                    permutation[fill_pos] = c;
                    col_used[c] = true;
                }
            }
        }

        if (cols < rows) {
            std::vector<rank_id> extend_permutation(rows, INVALID_RANK_ID);
            std::ranges::copy(permutation, extend_permutation.begin());
            std::iota(extend_permutation.begin() + static_cast<long>(cols),
                      extend_permutation.end(), cols);
            permutation = extend_permutation;
        }
        return permutation;
    }
}// namespace reshuffle::internal