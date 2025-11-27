#ifndef RESHUFFLE_GREEDY_RANK_ORDER_STRATEGY_HPP
#define RESHUFFLE_GREEDY_RANK_ORDER_STRATEGY_HPP

#include "rank_order_strategy.hpp"


namespace reshuffle::internal {

    class GreedyRankOrderStrategy final : public IOptimalRankOrderStrategy {
    public:
        [[nodiscard]] auto get_optimal_order(const Matrix2D<int> &matrix) const
                -> std::vector<RankId> override;
    };

}// namespace reshuffle::internal

#endif//RESHUFFLE_GREEDY_RANK_ORDER_STRATEGY_HPP
