#ifndef RESHUFFLE_RANK_REORDER_STRATEGY_HPP
#define RESHUFFLE_RANK_REORDER_STRATEGY_HPP

#include <context.hpp>
#include <vector>

namespace reshuffle::internal {

    template<typename T>
    using Matrix2D = std::vector<std::vector<T>>;

    class IOptimalRankOrderStrategy {
    public:
        virtual ~IOptimalRankOrderStrategy() = default;
        [[nodiscard]] virtual std::vector<RankId>
        get_optimal_order(const Matrix2D<int> &matrix) const = 0;
    };
}// namespace reshuffle::internal

#endif//RESHUFFLE_RANK_REORDER_STRATEGY_HPP
