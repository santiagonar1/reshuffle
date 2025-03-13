#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <mpi.h>
#include <numeric>
#include <ranges>
#include <span>
#include <vector>

#include "coloring.hpp"
#include "concepts.hpp"
#include "dimensions.hpp"
#include "mpi_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle {
    namespace internal {
        template<typename Tc, std::size_t N>
        auto split_equally(const std::span<Tc, N> values, const MPI_Comm &comm) {
            if (not is_root(comm) and not std::ranges::empty(values)) {
                throw std::invalid_argument("Only the root should have values!");
            }

            const int num_values = static_cast<int>(values.size());
            const auto num_ranks = get_num_ranks(comm);
            const auto new_distribution = make_block_wise(num_values, num_ranks);
            const auto new_global_coloring =
                    is_root(comm) ? get_global_coloring(new_distribution) : std::vector<rank_id>{};

            return scatter_from_root(values, comm, new_global_coloring);
        }

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

            if (in_mpi_comm(origin_comm)) {
                all_coloring = gather_in_root(std::span{local_coloring}, origin_comm);
                all_values = gather_in_root(values, origin_comm, old_global_coloring);
            }

            if (not in_mpi_comm(destiny_comm)) { return std::vector<T>{}; }

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
        template<typename Tc, std::size_t N>
        auto shuffle_with_coloring(const std::span<Tc, N> local_values, const MPI_Comm &comm,
                                   const BlockCyclic &old_distribution,
                                   const BlockCyclic &new_distribution) {
            const auto rank = internal::get_rank_id(comm);
            const auto num_ranks = internal::get_num_ranks(comm);

            const auto old_global_coloring = internal::get_global_coloring(old_distribution);
            const auto new_global_coloring = internal::get_global_coloring(new_distribution);

            const auto sending_coloring = internal::get_rank_ids_send_data_to(
                    old_global_coloring, new_global_coloring, rank);

            const auto receiving_coloring = internal::get_ranks_id_receive_data_from(
                    old_global_coloring, new_global_coloring, rank);

            // Steps:
            // 1. Get how many values to send to each rank
            // 2. Get how many values to receive from each rank
            // 3. Order local_values to send all values at once
            // 4. For each rank
            //       1. Send values to that rank (non-blocking
            //       2. Receive values from that rank
            //       3. Store all values in a receiver buffer
            //  5. Order the receiver buffer to match the expected order of values

            // 1
            auto num_values_send_per_rank = std::vector<int>(num_ranks);
            for (const auto rank_id: sending_coloring) { num_values_send_per_rank[rank_id]++; }

            // 2
            auto num_values_recv_per_rank = std::vector<int>(num_ranks);
            for (const auto rank_id: receiving_coloring) { num_values_recv_per_rank[rank_id]++; }

            // 3
            auto send_buffer = std::vector<int>(local_values.size());
            auto send_positions = std::vector<int>(num_ranks);
            std::exclusive_scan(num_values_send_per_rank.begin(), num_values_send_per_rank.end(),
                                send_positions.begin(), 0);
            for (int i = 0; i < sending_coloring.size(); i++) {
                const auto dest_rank = sending_coloring[i];
                send_buffer[send_positions[dest_rank]] = local_values[i];
                send_positions[dest_rank]++;
            }

            // 4
            auto recv_buffer = std::vector<std::remove_cv_t<Tc>>(receiving_coloring.size());
            std::exclusive_scan(num_values_send_per_rank.begin(), num_values_send_per_rank.end(),
                                send_positions.begin(), 0);
            auto recv_positions = std::vector<int>(num_ranks);
            std::exclusive_scan(num_values_recv_per_rank.begin(), num_values_recv_per_rank.end(),
                                recv_positions.begin(), 0);
            for (int i = 0; i < num_ranks; i++) {
                MPI_Request request{};
                MPI_Isend(send_buffer.data() + send_positions[i], num_values_send_per_rank[i],
                          internal::to_mpi_datatype<std::remove_cv_t<Tc>>(), i, 0, comm, &request);

                MPI_Recv(recv_buffer.data() + recv_positions[i], num_values_recv_per_rank[i],
                         internal::to_mpi_datatype<std::remove_cv_t<Tc>>(), i, 0, comm,
                         MPI_STATUS_IGNORE);

                MPI_Wait(&request, MPI_STATUS_IGNORE);
            }

            // 5
            auto recv_buffer_ordered = std::vector<std::remove_cv_t<Tc>>(receiving_coloring.size());
            for (int i = 0; i < receiving_coloring.size(); i++) {
                const auto src_rank = receiving_coloring[i];
                recv_buffer_ordered[i] = recv_buffer[recv_positions[src_rank]];
                recv_positions[src_rank]++;
            }

            return recv_buffer_ordered;
        }
    }// namespace dev

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &comm) {
        return internal::shuffle_with_coloring(std::span{values}, comm);
    }

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        const auto rank = internal::get_rank_id(comm);

        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_correct_num_values_provided(old_distribution, std::ranges::size(values),
                                                    rank);

        return dev::shuffle_with_coloring(std::span{values}, comm, old_distribution,
                                          new_distribution);
    }

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        internal::check_rank_only_in_destiny_comm_does_not_have_data(
                origin_comm, destiny_comm, not std::ranges::empty(values));

        return internal::shuffle_with_coloring(std::span{values}, origin_comm, destiny_comm);
    }

    template<concepts::ContiguousContainer C>
    auto shuffle(const C &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm,
                 const BlockCyclic &old_distribution, const BlockCyclic &new_distribution) {
        internal::check_distributions_have_same_num_values(old_distribution, new_distribution);
        internal::check_rank_only_in_destiny_comm_does_not_have_data(
                origin_comm, destiny_comm, not std::ranges::empty(values));

        if (internal::in_mpi_comm(origin_comm)) {
            const auto rank = internal::get_rank_id(origin_comm);
            internal::check_correct_num_values_provided(old_distribution, std::ranges::size(values),
                                                        rank);
        }

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);
        auto local_coloring = std::vector<rank_id>{};

        if (internal::in_mpi_comm(origin_comm)) {
            const auto rank = internal::get_rank_id(origin_comm);
            local_coloring = internal::get_global_and_local_coloring(old_global_coloring,
                                                                     new_distribution, rank)
                                     .local_coloring;
        }

        return internal::shuffle_with_coloring(std::span{values}, origin_comm, destiny_comm,
                                               local_coloring, old_global_coloring);
    }

    template<concepts::Iterable I>
        requires(not concepts::ContiguousContainer<I>)
    auto shuffle(const I &values, const MPI_Comm &comm) {
        using T = typename I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm);
    }

    template<concepts::Iterable I>
    auto shuffle(const I &values, const MPI_Comm &comm, const BlockCyclic &old_distribution,
                 const BlockCyclic &new_distribution) {
        using T = typename I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, comm, old_distribution, new_distribution);
    }

    template<concepts::Iterable I>
        requires(not concepts::ContiguousContainer<I>)
    auto shuffle(const I &values, const MPI_Comm &origin_comm, const MPI_Comm &destiny_comm) {
        using T = typename I::value_type;
        const std::vector<T> v_values(std::ranges::begin(values), std::ranges::end(values));
        return shuffle(v_values, origin_comm, destiny_comm);
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

        const auto rank = internal::get_rank_id(comm);
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

        if (internal::in_mpi_comm(origin_comm)) {
            const auto rank = internal::get_rank_id(origin_comm);
            const auto num_values = internal::num_elements(values);

            internal::check_correct_num_values_provided(old_distribution, num_values, rank);
        }

        const auto old_global_coloring = internal::get_global_coloring(old_distribution);
        const auto rank =
                internal::in_mpi_comm(destiny_comm) ? internal::get_rank_id(destiny_comm) : -1;

        const auto local_coloring = internal::in_mpi_comm(destiny_comm)
                                            ? internal::get_global_and_local_coloring(
                                                      old_global_coloring, new_distribution, rank)
                                                      .local_coloring
                                            : std::vector<rank_id>{};

        const auto subdomain_dimensions =
                internal::in_mpi_comm(destiny_comm)
                        ? internal::get_block_dimension(new_distribution, rank)
                        : internal::Dimension<2>{0, 0};

        auto buffer = std::vector<T>(std::ranges::join_view(values).begin(),
                                     std::ranges::join_view(values).end());
        buffer = internal::shuffle_with_coloring(std::span{buffer}, origin_comm, destiny_comm,
                                                 local_coloring, old_global_coloring);

        return internal::to_matrix(buffer, subdomain_dimensions);
    }
}// namespace reshuffle

#endif//RESHUFFLE_SHUFFLE_HPP
