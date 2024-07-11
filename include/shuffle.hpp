#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <algorithm>
#include <mpi.h>
#include <numeric>
#include <ranges>
#include <vector>

#include "concepts.hpp"
#include "dimensions.hpp"
#include "mpi_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle {
    namespace internal {
        template<concepts::ContiguousContainer C>
            requires concepts::Serializable<typename C::value_type>
        auto split_equally(const C &values, const MPI_Comm &comm) {
            if (not is_root(comm) and not std::ranges::empty(values)) {
                throw std::invalid_argument("Only the root should have values!");
            }

            const int num_values = static_cast<int>(values.size());
            const int rank{0};
            const auto num_ranks = internal::num_ranks(comm);
            const auto new_distribution = make_block_wise(num_values, num_ranks);
            const auto old_global_coloring =
                    is_root(comm) ? std::vector<rank_id>(num_values, 0) : std::vector<rank_id>{};
            const auto new_global_coloring =
                    is_root(comm) ? create_coloring(old_global_coloring, new_distribution, rank)
                                            .global_coloring
                                  : std::vector<rank_id>{};

            return internal::scatter_values_from_root(values, comm, new_global_coloring);
        }

        template<concepts::ContiguousContainer C>
            requires concepts::Serializable<typename C::value_type>
        auto shuffle_with_coloring(const C &values, const MPI_Comm &comm,
                                   const std::vector<rank_id> &local_coloring = {}) {
            auto all_coloring = internal::gather_values_in_root(local_coloring, comm);
            auto using_coloring = not all_coloring.empty();
            MPI_Bcast(&using_coloring, 1, MPI_CXX_BOOL, 0, comm);

            if (using_coloring and local_coloring.size() != std::ranges::size(values)) {
                throw std::invalid_argument(
                        "Coloring being used, but size of local_coloring vector does "
                        "not match size of data");
            }

            const auto all_values = internal::gather_values_in_root(values, comm);

            if (not using_coloring) { return internal::split_equally(all_values, comm); }

            return internal::scatter_values_from_root(all_values, comm, all_coloring);
        }

        template<concepts::ContiguousContainer C>
            requires concepts::Serializable<typename C::value_type>
        auto
        shuffle_with_coloring(const C &values, const MPI_Comm &origin_comm,
                              const MPI_Comm &destiny_comm,
                              const std::vector<rank_id> &local_coloring = std::vector<rank_id>{}) {
            using T = C::value_type;

            // TODO: Find way to check if root belongs to both communicators (or change algorithm)
            // Right now root is expected to belong to both origin and destiny communicators. We used
            // to have a check for this, but it was faulty. The largest issue was that it was using
            // MPI_COMM_WORLD, which did not work with Sessions and PSets.

            // TODO: What happens if both origin and destiny have data?
            const auto using_coloring = not local_coloring.empty();
            if (using_coloring and local_coloring.size() != std::ranges::size(values)) {
                throw std::invalid_argument("Coloring being used, but size of local_coloring "
                                            "vector does not match size of data");
            }

            auto all_coloring = std::vector<rank_id>{};
            auto all_values = std::vector<T>{};

            if (internal::in_mpi_comm(origin_comm)) {
                all_coloring = internal::gather_values_in_root(local_coloring, origin_comm);
                all_values = internal::gather_values_in_root(values, origin_comm);
            }

            if (not internal::in_mpi_comm(destiny_comm)) { return std::vector<T>{}; }

            // We need an additional variable in case rank 0 had no values to start with, but others
            // did, and those provided coloring.
            auto coloring_provided = not all_coloring.empty();
            MPI_Bcast(&coloring_provided, 1, MPI_CXX_BOOL, 0, destiny_comm);

            if (not coloring_provided) { return internal::split_equally(all_values, destiny_comm); }

            return internal::scatter_values_from_root(all_values, destiny_comm, all_coloring);
        }
    }// namespace internal

    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &comm) {
        return internal::shuffle_with_coloring(values, comm);
    }

    //TODO: Handle exceptions (compare sizes distributions and values).
    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        const auto rank = internal::get_rank_id(comm);
        const auto old_global_coloring = internal::get_global_coloring(old_distribution);

        const auto new_global_coloring =
                internal::is_root(MPI_COMM_WORLD)
                        ? internal::create_coloring(old_global_coloring, new_distribution, rank)
                                  .global_coloring
                        : std::vector<rank_id>{};

        return internal::scatter_values_from_root(internal::gather_values_in_root(values, comm),
                                                  comm, new_global_coloring);
    }

    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        return internal::shuffle_with_coloring(values, origin_comm, destiny_comm);
    }

    template<concepts::ContiguousContainer C>
        requires concepts::Serializable<typename C::value_type>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const BlockCyclic &old_distribution, const BlockCyclic &new_distribution) {
        using T = C::value_type;

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);

        const auto rank =
                internal::in_mpi_comm(destiny_comm) ? internal::get_rank_id(destiny_comm) : -1;

        const auto new_global_coloring =
                internal::is_root(destiny_comm)
                        ? internal::create_coloring(old_global_coloring, new_distribution, rank)
                                  .global_coloring
                        : std::vector<rank_id>{};

        const auto all_values = internal::in_mpi_comm(origin_comm)
                                        ? internal::gather_values_in_root(values, origin_comm)
                                        : std::vector<T>{};


        const auto my_values = internal::in_mpi_comm(destiny_comm)
                                       ? internal::scatter_values_from_root(
                                                 all_values, destiny_comm, new_global_coloring)
                                       : std::vector<T>{};

        return my_values;
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type> and
                 (not concepts::ContiguousContainer<I>)
    auto shuffle(const I &values, const MPI_Comm &comm) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm);
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm, old_distribution, new_distribution);
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type> and
                 (not concepts::ContiguousContainer<I>)
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm);
    }

    template<concepts::Iterable I>
        requires concepts::Serializable<typename I::value_type>
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const BlockCyclic &old_distribution, const BlockCyclic &new_distribution) {
        using T = I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm, old_distribution, new_distribution);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &comm,
                 const std::array<BlockCyclic, 2> &old_distribution,
                 const std::array<BlockCyclic, 2> &new_distribution) {
        using T = M::value_type::value_type;

        const auto rank = internal::get_rank_id(comm);
        const auto old_global_coloring = internal::get_global_coloring(old_distribution);

        const auto local_coloring =
                internal::create_coloring(old_global_coloring, new_distribution, rank)
                        .local_coloring;

        const auto subdomain_dimensions = internal::get_block_dimension(new_distribution, rank);

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = internal::shuffle_with_coloring(buffer, comm, local_coloring);

        return internal::to_matrix(buffer, subdomain_dimensions);
    }

    template<concepts::Matrix2D M>
    auto shuffle(const M &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const std::array<BlockCyclic, 2> &old_distribution,
                 const std::array<BlockCyclic, 2> &new_distribution) {
        using T = M::value_type::value_type;

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);

        const auto rank =
                internal::in_mpi_comm(destiny_comm) ? internal::get_rank_id(destiny_comm) : -1;

        const auto local_coloring =
                internal::in_mpi_comm(destiny_comm)
                        ? internal::create_coloring(old_global_coloring, new_distribution, rank)
                                  .local_coloring
                        : std::vector<rank_id>{};

        const auto subdomain_dimensions =
                internal::in_mpi_comm(destiny_comm)
                        ? internal::get_block_dimension(new_distribution, rank)
                        : internal::Dimension<2>{0, 0};

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = internal::shuffle_with_coloring(buffer, origin_comm, destiny_comm, local_coloring);

        return internal::to_matrix(buffer, subdomain_dimensions);
    }
}// namespace reshuffle

#endif//RESHUFFLE_SHUFFLE_HPP
