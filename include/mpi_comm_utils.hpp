#ifndef MPI_COMM_UTILS_HPP
#define MPI_COMM_UTILS_HPP

#include <mpi.h>
#include <ranges>

#include "coloring_utils.hpp"
#include "communication_package.hpp"
#include "intercommunicator.hpp"
#include "mpi_utils.hpp"
#include "rank_id.hpp"
#include "utils.hpp"

namespace reshuffle::internal {
    [[nodiscard]] auto get_displacements(const std::vector<int> &num_values_per_rank)
            -> std::vector<int>;

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto gather_in_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                      const MPI_Datatype &mpi_datatype,
                                      const std::vector<rank_id> &global_coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;

        const auto num_ranks = mpi::get_num_ranks(comm);
        const auto using_coloring = not global_coloring.empty();

        const int num_values{static_cast<int>(std::ranges::size(values))};
        std::vector<int> num_values_per_rank(num_ranks);
        MPI_Gather(&num_values, 1, MPI_INT, num_values_per_rank.data(), 1, MPI_INT, 0, comm);
        int total_num_values =
                std::accumulate(num_values_per_rank.cbegin(), num_values_per_rank.cend(), 0);

        auto all_values = mpi::is_root(comm) ? std::vector<T>(total_num_values) : std::vector<T>{};
        const auto displacements = get_displacements(num_values_per_rank);

        MPI_Gatherv(std::ranges::data(values), num_values, mpi_datatype, all_values.data(),
                    num_values_per_rank.data(), displacements.data(), mpi_datatype, 0, comm);

        if (using_coloring and mpi::is_root(comm)) {
            const auto indices = get_global_index_by_rank(global_coloring, num_ranks);
            all_values = reorder_values(all_values, indices);
        }

        return all_values;
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto gather_in_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                      const std::vector<rank_id> &global_coloring = {})
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
        MPI_Datatype mpi_datatype = mpi::to_mpi_datatype<T>();

        return gather_in_root(values, comm, mpi_datatype, global_coloring);
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto scatter_from_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                         const MPI_Datatype &mpi_datatype,
                                         const std::vector<rank_id> &coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
        int num_ranks{};
        int rank{};
        const auto using_coloring = not coloring.empty();

        const int total_num_values = static_cast<int>(values.size());


        if (using_coloring and coloring.size() != total_num_values) {
            throw std::invalid_argument(
                    "Coloring being used, but size of coloring vector does not match size of data");
        }

        MPI_Comm_size(comm, &num_ranks);
        MPI_Comm_rank(comm, &rank);

        auto new_num_values_per_rank =
                mpi::is_root(comm) ? internal::calc_num_values_per_rank(num_ranks, coloring)
                                   : std::vector<int>(num_ranks);

        MPI_Bcast(new_num_values_per_rank.data(), static_cast<int>(new_num_values_per_rank.size()),
                  MPI_INT, 0, comm);

        const auto displacements = internal::get_displacements(new_num_values_per_rank);
        const int new_num_values = new_num_values_per_rank[rank];

        auto values_to_scatter = using_coloring ? order_by_color(values, coloring, displacements)
                                                : std::vector<T>(std::ranges::begin(values),
                                                                 std::ranges::end(values));

        auto my_values = std::vector<T>(new_num_values);
        MPI_Scatterv(values_to_scatter.data(), new_num_values_per_rank.data(), displacements.data(),
                     mpi_datatype, my_values.data(), new_num_values, mpi_datatype, 0, comm);

        return my_values;
    }

    template<typename Tc, std::size_t N>
    [[nodiscard]] auto scatter_from_root(const std::span<Tc, N> values, const MPI_Comm &comm,
                                         const std::vector<rank_id> &coloring)
            -> std::vector<std::remove_cv_t<Tc>> {
        using T = std::remove_cv_t<Tc>;
        MPI_Datatype mpi_datatype = mpi::to_mpi_datatype<T>();

        return scatter_from_root(values, comm, mpi_datatype, coloring);
    }

    template<typename T>
    [[nodiscard]] auto exchange_values(const std::vector<T> &values,
                                       const std::vector<rank_id> &sending_ids,
                                       const std::vector<rank_id> &recv_ids, const MPI_Comm &comm)
            -> std::vector<T> {
        // Steps:
        // 1. Get how many values to send to each rank
        // 2. Get how many values to receive from each rank
        // 3. Order local_values to send all values at once
        // 4. For each rank
        //       1. Send values to that rank
        //       2. Receive values from that rank
        //       3. Store all values in a receiver buffer
        //  5. Order the receiver buffer to match the expected order of values

        const auto num_ranks = mpi::get_num_ranks(comm);

        // 1
        const auto num_values_send_per_rank = get_num_repetitions(sending_ids, num_ranks - 1);

        // 2
        const auto num_values_recv_per_rank = get_num_repetitions(recv_ids, num_ranks - 1);

        // 3
        auto send_buffer = group_values_by_rank_id(values, sending_ids, num_ranks);

        // 4
        auto send_positions = std::vector<int>(num_ranks);
        std::exclusive_scan(num_values_send_per_rank.begin(), num_values_send_per_rank.end(),
                            send_positions.begin(), 0);

        auto recv_positions = std::vector<int>(num_ranks);
        std::exclusive_scan(num_values_recv_per_rank.begin(), num_values_recv_per_rank.end(),
                            recv_positions.begin(), 0);

        auto requests = std::vector<MPI_Request>(num_ranks);
        auto recv_buffer = std::vector<std::remove_cv_t<T>>(recv_ids.size());
        for (int i = 0; i < num_ranks; i++) {
            MPI_Isendrecv(send_buffer.data() + send_positions[i], num_values_send_per_rank[i],
                          mpi::to_mpi_datatype<std::remove_cv_t<T>>(), i, 0,
                          recv_buffer.data() + recv_positions[i], num_values_recv_per_rank[i],
                          mpi::to_mpi_datatype<std::remove_cv_t<T>>(), i, 0, comm,
                          requests.data() + i);
        }

        MPI_Waitall(num_ranks, requests.data(), MPI_STATUSES_IGNORE);

        // 5
        auto recv_buffer_ordered = std::vector<std::remove_cv_t<T>>(recv_ids.size());
        for (int i = 0; i < recv_ids.size(); i++) {
            const auto src_rank = recv_ids[i];
            recv_buffer_ordered[i] = recv_buffer[recv_positions[src_rank]];
            recv_positions[src_rank]++;
        }

        return recv_buffer_ordered;
    }
}// namespace reshuffle::internal

namespace reshuffle::dev::internal {

    template<typename T>
    auto async_send(std::span<T> send_buffer, const std::vector<Block> &grouped_blocks_to_send,
                    const reshuffle::internal::Intercommunicator &intercomm)
            -> std::vector<MPI_Request> {
        const auto num_ranks_to_send = static_cast<int>(grouped_blocks_to_send.size());
        auto send_requests = std::vector<MPI_Request>(num_ranks_to_send);
        const auto comm = intercomm.get_intercommunicator();

        for (int i = 0; i < num_ranks_to_send; i++) {
            const auto &block = grouped_blocks_to_send[i];
            const auto num_values = block.get_interval().get_length();
            // block.get_owner() in this context will return a rank relative to the final comm.
            // Thus, this should never fail. If it throws, something went wrong.
            const auto rank_id_destiny_final_comm = block.get_owner();
            const auto destiny = intercomm.get_intercomm_rank(
                    rank_id_destiny_final_comm,
                    reshuffle::internal::Intercommunicator::SelectCommunicator::FINAL_COMM);

            MPI_Isend(send_buffer.data(), num_values, mpi::to_mpi_datatype<std::remove_cv_t<T>>(),
                      destiny, 0, comm, &send_requests[i]);

            send_buffer = send_buffer | std::views::drop(num_values);
        }

        return send_requests;
    }

    template<typename T>
    auto async_receive(std::span<T> receive_buffer,
                       const std::vector<Block> &grouped_blocks_to_receive,
                       const reshuffle::internal::Intercommunicator &intercomm)
            -> std::vector<MPI_Request> {
        const auto num_ranks_to_receive = static_cast<int>(grouped_blocks_to_receive.size());
        auto receive_requests = std::vector<MPI_Request>(num_ranks_to_receive);
        const auto comm = intercomm.get_intercommunicator();

        for (int i = 0; i < num_ranks_to_receive; i++) {
            const auto &block = grouped_blocks_to_receive[i];
            const auto num_values = block.get_interval().get_length();
            // block.get_owner() in this context will return a rank relative to the initial comm.
            // Thus, this should never fail. If it throws, something went wrong.
            const auto rank_id_source_initial_comm = block.get_owner();
            const auto source = intercomm.get_intercomm_rank(
                    rank_id_source_initial_comm,
                    reshuffle::internal::Intercommunicator::SelectCommunicator::INITIAL_COMM);

            MPI_Irecv(receive_buffer.data(), num_values,
                      mpi::to_mpi_datatype<std::remove_cv_t<T>>(), source, MPI_ANY_TAG, comm,
                      &receive_requests[i]);

            receive_buffer = receive_buffer | std::views::drop(num_values);
        }

        return receive_requests;
    }

    template<typename T>
    auto exchange_values(std::span<const T> local_values, const std::vector<Block> &blocks_to_send,
                         const std::vector<Block> &blocks_to_receive,
                         const reshuffle::internal::Intercommunicator &intercomm)
            -> std::vector<T> {
        auto [send_buffer, grouped_blocks_to_send] =
                internal::get_send_package(local_values, blocks_to_send);
        auto [receive_buffer, grouped_blocks_to_receive] =
                internal::get_receive_package<T>(blocks_to_receive);

        auto send_requests = async_send(std::span{send_buffer}, grouped_blocks_to_send, intercomm);
        auto receive_requests =
                async_receive(std::span{receive_buffer}, grouped_blocks_to_receive, intercomm);

        const auto num_ranks_to_receive = static_cast<int>(receive_requests.size());

        auto new_local_values = std::vector<T>(receive_buffer.size());
        for (int i = 0; i < num_ranks_to_receive; i++) {
            auto source{MPI_PROC_NULL};

            MPI_Waitany(num_ranks_to_receive, receive_requests.data(), &source, MPI_STATUS_IGNORE);
            const auto &received_block = grouped_blocks_to_receive[source];
            const auto message_starts = received_block.get_interval().get_left_bound();
            const auto num_values = static_cast<size_t>(received_block.get_interval().get_length());

            auto blocks_from_source = blocks_to_receive | std::views::filter([source](auto block) {
                                          return block.get_owner() == source;
                                      });

            auto received_values = std::span{receive_buffer.begin() + message_starts, num_values};
            for (const auto &block: blocks_from_source) {
                const auto num_values_in_block = block.get_interval().get_length();
                const auto start_local = block.get_interval().get_left_bound();

                std::copy(received_values.begin(), received_values.begin() + num_values_in_block,
                          new_local_values.begin() + start_local);

                received_values = received_values | std::views::drop(num_values_in_block);
            }
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);

        return new_local_values;
    }
}// namespace reshuffle::dev::internal

#endif//MPI_COMM_UTILS_HPP
