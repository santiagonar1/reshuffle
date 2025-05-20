#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <mpi.h>
#include <ranges>
#include <span>
#include <vector>

#include "coloring.hpp"
#include "communication_package.hpp"
#include "concepts.hpp"
#include "context.hpp"
#include "dimensions.hpp"
#include "mpi_comm_utils.hpp"
#include "mpi_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle {
    namespace internal {
        template<typename Tc, std::size_t N>
        auto split_equally(const std::span<Tc, N> values, const MPI_Comm &comm) {
            if (not mpi::is_root(comm) and not std::ranges::empty(values)) {
                throw std::invalid_argument("Only the root should have values!");
            }

            const int num_values = static_cast<int>(values.size());
            const auto num_ranks = mpi::get_num_ranks(comm);
            const auto new_distribution = make_block_wise(num_values, num_ranks);
            const auto new_global_coloring = mpi::is_root(comm)
                                                     ? get_global_coloring(new_distribution)
                                                     : std::vector<rank_id>{};

            return scatter_from_root(values, comm, new_global_coloring);
        }

        template<concepts::ContiguousContainer C>
        auto shuffle_with_coloring(const C &local_values, const MPI_Comm &comm,
                                   const BlockCyclic &old_distribution,
                                   const BlockCyclic &new_distribution) {
            const auto rank = mpi::get_rank_id(comm);

            const auto old_global_coloring = get_global_coloring(old_distribution);
            const auto new_global_coloring = get_global_coloring(new_distribution);

            const auto sending_data_to =
                    get_rank_ids_send_data_to(old_global_coloring, new_global_coloring, rank);

            const auto receiving_data_from =
                    get_ranks_id_receive_data_from(old_global_coloring, new_global_coloring, rank);

            return exchange_values(local_values, sending_data_to, receiving_data_from, comm);
        }

        // TODO: Remove eventually, but right now being used by 2D implementations
        template<typename Tc, std::size_t N>
        auto shuffle_with_coloring(const std::span<Tc, N> values, const MPI_Comm &comm,
                                   const std::vector<rank_id> &local_coloring = {},
                                   const std::vector<rank_id> &old_global_coloring = {}) {
            auto all_coloring =
                    gather_in_root(std::span{local_coloring}, comm, old_global_coloring);
            auto using_coloring = not all_coloring.empty();
            MPI_Bcast(&using_coloring, 1, MPI_CXX_BOOL, 0, comm);

            if (using_coloring and local_coloring.size() != std::ranges::size(values)) {
                throw std::invalid_argument(
                        "Coloring being used, but size of local_coloring vector does "
                        "not match size of data");
            }

            const auto all_values = gather_in_root(values, comm, old_global_coloring);

            if (not using_coloring) { return split_equally(std::span{all_values}, comm); }

            return scatter_from_root(std::span{all_values}, comm, all_coloring);
        }

        template<typename Tc, std::size_t N>
        auto shuffle_with_coloring(const std::span<Tc, N> values, const MPI_Comm &origin_comm,
                                   const MPI_Comm &destiny_comm,
                                   const std::vector<rank_id> &local_coloring = {},
                                   const std::vector<rank_id> &old_global_coloring = {}) {
            using T = std::remove_cv_t<Tc>;

            // TODO: Find way to check if root belongs to both communicators (or change algorithm)
            // Right now root is expected to belong to both origin and destiny communicators. We used
            // to have a check for this, but it was faulty. The largest issue was that it was using
            // MPI_COMM_WORLD, which did not work with Sessions and PSets.

            const auto using_coloring = not local_coloring.empty();
            if (using_coloring and local_coloring.size() != std::ranges::size(values)) {
                throw std::invalid_argument("Coloring being used, but size of local_coloring "
                                            "vector does not match size of data");
            }

            auto all_coloring = std::vector<rank_id>{};
            auto all_values = std::vector<T>{};

            if (mpi::in_mpi_comm(origin_comm)) {
                all_coloring = gather_in_root(std::span{local_coloring}, origin_comm);
                all_values = gather_in_root(values, origin_comm, old_global_coloring);
            }

            if (not mpi::in_mpi_comm(destiny_comm)) { return std::vector<T>{}; }

            // We need an additional variable in case rank 0 had no values to start with, but others
            // did, and those provided coloring.
            auto coloring_provided = not all_coloring.empty();
            MPI_Bcast(&coloring_provided, 1, MPI_CXX_BOOL, 0, destiny_comm);

            if (not coloring_provided) {
                return split_equally(std::span{all_values}, destiny_comm);
            }

            return scatter_from_root(std::span{all_values}, destiny_comm, all_coloring);
        }

        void check_distributions_have_same_num_values(const BlockCyclic &d1, const BlockCyclic &d2);

        void check_distributions_have_same_num_values(const std::array<BlockCyclic, 2> &d1,
                                                      const std::array<BlockCyclic, 2> &d2);

        void check_correct_num_values_provided(const BlockCyclic &distribution, int num_values,
                                               rank_id rank);
        void check_correct_num_values_provided(const std::array<BlockCyclic, 2> &distribution,
                                               int num_values, rank_id rank);

        void check_rank_only_in_destiny_comm_does_not_have_data(const MPI_Comm &origin_comm,
                                                                const MPI_Comm &destiny_comm,
                                                                bool contains_data);
    }// namespace internal

    namespace dev {
        namespace internal {

            // TODO: move to utils after old code has been removed
            template<typename T>
            auto remove_duplicates(const std::vector<T> &values) -> std::vector<T> {
                auto unique_values = values;
                std::sort(unique_values.begin(), unique_values.end());
                auto [new_end, _] = std::ranges::unique(unique_values);
                unique_values.erase(new_end, unique_values.end());
                return unique_values;
            }

            // TODO: I think this function should return a pair of vectors of MultiBlock
            // I have not changed it yet to avoid having to modify the exchange, but that should
            // be my next modification
            template<std::size_t N>
            auto get_send_and_receive_blocks(
                    const GridOverlay<N> &grid_overlay,
                    const reshuffle::internal::Coordinates<N> &rank_initial_grid,
                    const reshuffle::internal::Coordinates<N> &rank_final_grid)
                    -> std::pair<std::array<std::vector<Block>, N>,
                                 std::array<std::vector<Block>, N>> {
                auto send_blocks = std::array<std::vector<Block>, N>{};
                auto receive_blocks = std::array<std::vector<Block>, N>{};

                const auto &multidimensional_blocks =
                        grid_overlay.get_grid().get_multidimensional_blocks();
                const auto &coordinate_owners_target =
                        grid_overlay.get_coordinates_owners_target_grid();

                for (int i = 0; i < multidimensional_blocks.size(); ++i) {
                    const auto owner_initial_grid =
                            get_owner_coordinates(multidimensional_blocks[i]);
                    const auto &owner_target_grid = coordinate_owners_target[i];

                    // The owners in the send_blocks are relative to the final grid
                    if (owner_initial_grid == rank_initial_grid) {
                        for (int dim = 0; dim < N; ++dim) {
                            const auto &block = multidimensional_blocks[i][dim];
                            send_blocks[dim].emplace_back(block.get_interval(),
                                                          owner_target_grid[dim]);
                        }
                    }

                    // The owners in the receive_blocks are relative to the initial grid
                    if (owner_target_grid == rank_final_grid) {
                        for (int dim = 0; dim < N; ++dim) {
                            const auto &block = multidimensional_blocks[i][dim];
                            receive_blocks[dim].emplace_back(block);
                        }
                    }
                }

                std::ranges::transform(send_blocks, send_blocks.begin(),
                                       [](const auto &block_vector) {
                                           return join(remove_duplicates(block_vector));
                                       });


                std::ranges::transform(receive_blocks, receive_blocks.begin(),
                                       [](const auto &block_vector) {
                                           return join(remove_duplicates(block_vector));
                                       });


                return {send_blocks, receive_blocks};
            }
        }// namespace internal

        template<typename T, typename Extents>
        auto get_dimensions(std::mdspan<const T, Extents> local_values)
                -> Dimensions<Extents::rank()> {
            constexpr auto N = Extents::rank();

            if (local_values.empty()) { return Dimensions<N>{}; }

            auto dimensions = Dimensions<N>{};
            for (int i = 0; i < N; ++i) { dimensions[i] = local_values.extent(i); }

            return dimensions;
        }

        // TODO: I probably need to return the local dimensions as well
        template<typename T, typename Extents>
        auto shuffle(std::mdspan<const T, Extents> local_values,
                     const Context<Extents::rank()> &initial_context,
                     const Context<Extents::rank()> &final_context)
                -> std::pair<std::vector<T>, Dimensions<Extents::rank()>>
            requires(Extents::rank() <= 3)
        {
            if (initial_context == final_context) {
                return {get_1D_data(local_values), get_dimensions(local_values)};
            }

            const auto grid_overlay = initial_context.distribution.get_grid_layout().get_overlay(
                    final_context.distribution.get_grid_layout(),
                    final_context.distribution.get_processor_grid());

            // TODO: Deal with partially disjoint communicators
            const auto intercomm = reshuffle::internal::Intercommunicator(initial_context.comm,
                                                                          final_context.comm);

            const auto comm = intercomm.get_intercommunicator();
            const auto rank_intercomm = mpi::get_rank_id(comm);
            const auto rank_initial_grid =
                    intercomm.get_initial_comm_rank(rank_intercomm).value_or(-1);
            const auto rank_final_grid = intercomm.get_final_comm_rank(rank_intercomm).value_or(-1);

            const auto initial_processor_grid = initial_context.distribution.get_processor_grid();
            const auto final_processor_grid = final_context.distribution.get_processor_grid();

            const auto rank_coordinates_initial_grid =
                    initial_processor_grid.get_processor_coordinates(rank_initial_grid);
            const auto rank_coordinates_final_grid =
                    final_processor_grid.get_processor_coordinates(rank_final_grid);

            const auto [blocks_to_send, blocks_to_receive] = internal::get_send_and_receive_blocks(
                    grid_overlay, rank_coordinates_initial_grid, rank_coordinates_final_grid);

            return internal::exchange_values(local_values, blocks_to_send, blocks_to_receive,
                                             initial_processor_grid, final_processor_grid,
                                             intercomm);
        }
    }// namespace dev

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        const auto rank = mpi::get_rank_id(comm);

        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_correct_num_values_provided(old_distribution, std::ranges::size(values),
                                                    rank);

        if (old_distribution == new_distribution) { return values; }

        return internal::shuffle_with_coloring(values, comm, old_distribution, new_distribution);
    }

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const BlockCyclic &old_distribution, const BlockCyclic &new_distribution) {
        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_rank_only_in_destiny_comm_does_not_have_data(
                origin_comm, destiny_comm, not std::ranges::empty(values));

        if (mpi::in_mpi_comm(origin_comm)) {
            const auto rank = mpi::get_rank_id(origin_comm);
            internal::check_correct_num_values_provided(old_distribution, std::ranges::size(values),
                                                        rank);
        }

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);
        auto local_coloring = std::vector<rank_id>{};

        if (mpi::in_mpi_comm(origin_comm)) {
            const auto rank = mpi::get_rank_id(origin_comm);
            local_coloring = internal::get_global_and_local_coloring(old_global_coloring,
                                                                     new_distribution, rank)
                                     .local_coloring;
        }

        return internal::shuffle_with_coloring(std::span{values}, origin_comm, destiny_comm,
                                               local_coloring, old_global_coloring);
    }

    template<concepts::Iterable I>
    auto shuffle(const I &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        using T = typename I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm, old_distribution, new_distribution);
    }

    template<concepts::Iterable I>
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const BlockCyclic &old_distribution, const BlockCyclic &new_distribution) {
        using T = typename I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm, old_distribution, new_distribution);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &comm,
                 const std::array<BlockCyclic, 2> &old_distribution,
                 const std::array<BlockCyclic, 2> &new_distribution) {
        using T = typename M::value_type::value_type;

        const auto rank = mpi::get_rank_id(comm);
        const auto num_values = internal::num_elements(values);

        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_correct_num_values_provided(old_distribution, num_values, rank);

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);

        const auto local_coloring =
                internal::get_global_and_local_coloring(old_global_coloring, new_distribution, rank)
                        .local_coloring;

        const auto subdomain_dimensions = internal::get_block_dimension(new_distribution, rank);

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = internal::shuffle_with_coloring(std::span{buffer}, comm, local_coloring,
                                                 old_global_coloring);

        return internal::to_matrix(buffer, subdomain_dimensions);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::array<BlockCyclic, 2> &old_distribution,
                 const std::array<BlockCyclic, 2> &new_distribution) {
        using T = typename M::value_type::value_type;

        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_rank_only_in_destiny_comm_does_not_have_data(
                origin_comm, destiny_comm, not std::ranges::empty(values));

        if (mpi::in_mpi_comm(origin_comm)) {
            const auto rank = mpi::get_rank_id(origin_comm);
            const auto num_values = internal::num_elements(values);

            internal::check_correct_num_values_provided(old_distribution, num_values, rank);
        }

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);
        const auto rank = mpi::in_mpi_comm(destiny_comm) ? mpi::get_rank_id(destiny_comm) : -1;

        const auto local_coloring = mpi::in_mpi_comm(destiny_comm)
                                            ? internal::get_global_and_local_coloring(
                                                      old_global_coloring, new_distribution, rank)
                                                      .local_coloring
                                            : std::vector<rank_id>{};

        const auto subdomain_dimensions =
                mpi::in_mpi_comm(destiny_comm)
                        ? internal::get_block_dimension(new_distribution, rank)
                        : Dimensions{0, 0};

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = internal::shuffle_with_coloring(std::span{buffer}, origin_comm, destiny_comm,
                                                 local_coloring, old_global_coloring);

        return internal::to_matrix(buffer, subdomain_dimensions);
    }
}// namespace reshuffle

#endif//RESHUFFLE_SHUFFLE_HPP
