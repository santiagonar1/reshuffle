#ifndef RANK_ORDER_HPP
#define RANK_ORDER_HPP

#include "context.hpp"
#include "grid_overlay.hpp"
#include "mpi_utils.hpp"
#include "rank_order_strategy.hpp"

#include <iomanip>
#include <iostream>
#include <vector>
namespace reshuffle::internal {

    template<typename T>
    using Matrix2D = std::vector<std::vector<T>>;

    template<std::size_t N>
    class RankOrder {


    public:
        RankOrder(const distribution::DataDistribution<N> &initial_distribution,
                  const distribution::DataDistribution<N> &final_distribution,
                  const IOptimalRankOrderStrategy &strategy);

        [[nodiscard]] auto get_matrix() const -> const Matrix2D<int> &;

#ifdef RESHUFFLE_TESTING
        void _test_set_matrix(Matrix2D<int> m);
#endif

        [[nodiscard]] auto get_optimal_rank_order() const -> std::vector<RankId>;

        static auto get_reordered_comm(const MPI_Comm &comm, const std::vector<RankId> &new_order)
                -> MPI_Comm;

        auto
        compute_communication_weights(const distribution::DataDistribution<N> &initial_distribution,
                                      const distribution::DataDistribution<N> &final_distribution)
                -> Matrix2D<int>;


    private:
        //a matrix, which maps all communication between ranks:
        //_matrix[0][0] -> communication weight from rank 0 to rank 0
        //_matrix[0][1] -> communication weight from rank 0 to rank 1
        Matrix2D<int> _matrix{};
        const IOptimalRankOrderStrategy &_strategy;
    };

    template<std::size_t N>
    RankOrder<N>::RankOrder(const distribution::DataDistribution<N> &initial_distribution,
                            const distribution::DataDistribution<N> &final_distribution,
                            const IOptimalRankOrderStrategy &strategy)
        : _strategy(strategy) {
        _matrix = compute_communication_weights(initial_distribution, final_distribution);
    }

    template<std::size_t N>
    auto RankOrder<N>::get_matrix() const -> const Matrix2D<int> & {
        return _matrix;
    }

#ifdef RESHUFFLE_TESTING
    template<std::size_t N>
    void RankOrder<N>::_test_set_matrix(Matrix2D<int> m) {
        _matrix = std::move(m);
    }
#endif


    template<std::size_t N>
    auto RankOrder<N>::get_optimal_rank_order() const -> std::vector<RankId> {
        return _strategy.get_optimal_order(_matrix);
    }

    template<std::size_t N>
    auto RankOrder<N>::get_reordered_comm(const MPI_Comm &comm,
                                          const std::vector<RankId> &new_order) -> MPI_Comm {

        const auto final_rank_phys = mpi::get_rank_id(comm).value();

        std::vector<RankId> position_in_final_communicator(new_order.size(), -1);
        for (int i = 0; i < position_in_final_communicator.size(); ++i) {
            const auto r = new_order[i];
            position_in_final_communicator[r] = i;
        }

        auto reordered_comm_final = MPI_COMM_NULL;
        const RankId key = position_in_final_communicator[final_rank_phys];
        MPI_Comm_split(comm, 0, key, &reordered_comm_final);

        return reordered_comm_final;
    }

    template<std::size_t N>
    auto RankOrder<N>::compute_communication_weights(
            const distribution::DataDistribution<N> &initial_distribution,
            const distribution::DataDistribution<N> &final_distribution) -> Matrix2D<int> {
        auto communicationWeightMatrix = Matrix2D<int>(
                initial_distribution.get_processor_grid().get_num_processors(),
                std::vector<int>(final_distribution.get_processor_grid().get_num_processors(), 0));
        const auto &initial_grid = initial_distribution.get_grid_layout();
        const auto &final_grid = final_distribution.get_grid_layout();
        const auto initialProcessorGrid = initial_distribution.get_processor_grid();
        const auto finalProcessorGrid = final_distribution.get_processor_grid();

        for (const auto initialOverlay = GridOverlay{initial_grid, final_grid};
             auto [initialMultiBlock, target_owner_coordinates]:
             std::views::zip(initialOverlay.get_multidimensional_blocks_origin(),
                             initialOverlay.get_coordinates_owners_target_grid())) {
            const auto block_size = internal::get_num_elements(initialMultiBlock);
            const auto owner_coordinates = internal::get_owner_coordinates(initialMultiBlock);
            const auto block_owner = internal::map_indices(
                    owner_coordinates, initial_distribution.get_processor_grid().get_dimensions());
            const auto block_target =
                    internal::map_indices(target_owner_coordinates,
                                          final_distribution.get_processor_grid().get_dimensions());
            communicationWeightMatrix[block_owner.value()][block_target.value()] += block_size;
        }
        return communicationWeightMatrix;
    }

    template<std::size_t N>
    RankOrder(const Context<N> &, const Context<N> &, IOptimalRankOrderStrategy &) -> RankOrder<N>;

}// namespace reshuffle::internal

#endif//RANK_ORDER_HPP
