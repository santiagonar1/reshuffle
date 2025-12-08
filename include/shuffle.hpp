#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <mpi.h>
#include <ranges>
#include <vector>

#include "communication_package.hpp"
#include "concepts.hpp"
#include "context.hpp"
#include "data_exchanger.hpp"
#include "dimensions.hpp"
#include "general_data_exchanger.hpp"
#include "greedy_rank_order_strategy.hpp"
#include "hungarian_rank_order_strategy.hpp"
#include "mpi_comm_utils.hpp"
#include "profiler.hpp"
#include "rank_order.hpp"
#include "scatter_exchanger.hpp"
#include "utils.hpp"


namespace reshuffle {
    namespace internal {
        template<typename T, typename Extents>
        auto select_exchanger(std::mdspan<const T, Extents> local_values,
                              const Context<Extents::rank()> &initial_context,
                              const Context<Extents::rank()> &final_context)
                -> std::unique_ptr<DataExchanger<T, Extents::rank()>> {
            if (initial_context.distribution.get_processor_grid().get_num_processors() == 1 and
                final_context.distribution.is_block_wise()) {
                return std::make_unique<ScatterExchanger<T, Extents>>(local_values, initial_context,
                                                                      final_context);
            }

            return std::make_unique<GeneralDataExchanger<T, Extents>>(local_values, initial_context,
                                                                      final_context);
        }
    }// namespace internal

    template<concepts::Exchangeable T, typename Extents>
    auto shuffle(std::mdspan<const T, Extents> local_values,
                 const Context<Extents::rank()> &initial_context,
                 const Context<Extents::rank()> &final_context)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>>
        requires(Extents::rank() <= 3)
    {
        PROFILE_SCOPE_NAMED("Shuffle");
        if (initial_context == final_context) {
            return {internal::get_1D_data(local_values), internal::get_dimensions(local_values)};
        }

        const auto data_exchanger = internal::select_exchanger<T, Extents>(
                local_values, initial_context, final_context);

        return data_exchanger->exchange();
    }

    template<typename T>
    auto shuffle(const std::vector<std::vector<T>> &local_values, const Context<2> &initial_context,
                 const Context<2> &final_context) -> std::pair<std::vector<T>, Dimensions<2>> {
        auto flat_data = internal::to_vector(local_values);
        const auto dimensions = internal::get_dimensions(local_values);
        return shuffle(std::mdspan(std::as_const(flat_data).data(), dimensions), initial_context,
                       final_context);
    }

    template<std::size_t N>
    auto get_optimal_communicator(const Context<N> &initial_context,
                                  const Context<N> &final_context)
            -> std::optional<std::pair<MPI_Comm, std::vector<RankId>>> {
        const auto commWeight =
                internal::RankOrder<N>(initial_context.distribution, final_context.distribution,
                                       internal::HungarianRankOrderStrategy{});
        const auto reordering = commWeight.get_optimal_rank_order();
        return std::make_optional(std::make_pair(
                internal::RankOrder<N>::get_reordered_comm(final_context.comm, reordering),
                reordering));
    }

    template<std::size_t N>
    auto get_optimal_communicator_greedy(const Context<N> &initial_context,
                                         const Context<N> &final_context)
            -> std::optional<std::pair<MPI_Comm, std::vector<RankId>>> {
        const auto commWeight =
                internal::RankOrder<N>(initial_context.distribution, final_context.distribution,
                                       internal::GreedyRankOrderStrategy{});
        const auto reordering = commWeight.get_optimal_rank_order();
        return std::make_optional(std::make_pair(
                internal::RankOrder<N>::get_reordered_comm(final_context.comm, reordering),
                reordering));
    }
}// namespace reshuffle


#endif//RESHUFFLE_SHUFFLE_HPP
