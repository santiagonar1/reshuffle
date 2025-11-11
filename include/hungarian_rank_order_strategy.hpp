#ifndef RESHUFFLE_HUNGARIAN_RANK_ORDER_STRATEGY_HPP
#define RESHUFFLE_HUNGARIAN_RANK_ORDER_STRATEGY_HPP


#include "rank_order_strategy.hpp"

namespace reshuffle::internal {

    class HungarianRankOrderStrategy final : public IOptimalRankOrderStrategy {
    public:
        [[nodiscard]] auto get_optimal_order(const Matrix2D<int> &matrix) const
                -> std::vector<rank_id> override;
    };

}// namespace reshuffle::internal

#endif//RESHUFFLE_HUNGARIAN_RANK_ORDER_STRATEGY_HPP
