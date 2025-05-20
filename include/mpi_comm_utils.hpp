#ifndef MPI_COMM_UTILS_HPP
#define MPI_COMM_UTILS_HPP

#include <mpi.h>
#include <ranges>

#include "cartesian_product.hpp"
#include "communication_package.hpp"
#include "intercommunicator.hpp"
#include "mpi_utils.hpp"

namespace reshuffle::internal {

    template<typename T>
    auto async_send(std::span<T> send_buffer, const std::vector<Block> &grouped_blocks_to_send,
                    const Intercommunicator &intercomm) -> std::vector<MPI_Request> {
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
                    rank_id_destiny_final_comm, Intercommunicator::SelectCommunicator::FINAL_COMM);

            MPI_Isend(send_buffer.data(), num_values, mpi::to_mpi_datatype<std::remove_cv_t<T>>(),
                      destiny, 0, comm, &send_requests[i]);

            send_buffer = send_buffer | std::views::drop(num_values);
        }

        return send_requests;
    }

    template<typename T>
    auto async_receive(std::span<T> receive_buffer,
                       const std::vector<Block> &grouped_blocks_to_receive,
                       const Intercommunicator &intercomm) -> std::vector<MPI_Request> {
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
                    Intercommunicator::SelectCommunicator::INITIAL_COMM);

            MPI_Irecv(receive_buffer.data(), num_values,
                      mpi::to_mpi_datatype<std::remove_cv_t<T>>(), source, MPI_ANY_TAG, comm,
                      &receive_requests[i]);

            receive_buffer = receive_buffer | std::views::drop(num_values);
        }

        return receive_requests;
    }

    template<std::size_t N>
    auto get_dimensions(const std::array<std::vector<Block>, N> &blocks_to_receive)
            -> Dimensions<N> {

        if (blocks_to_receive[0].empty()) { return std::array<int, N>{}; }

        std::array<int, N> dimensions{};
        for (int i = 0; i < N; i++) {
            dimensions[i] = blocks_to_receive[i].back().get_interval().get_right_bound();
        }
        return dimensions;
    }

    template<typename T, typename Extents>
    auto exchange_values(std::mdspan<const T, Extents> local_values,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_send,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_receive,
                         const ProcessorGrid<Extents::rank()> &initial_processor_grid,
                         const ProcessorGrid<Extents::rank()> &final_processor_grid,
                         const Intercommunicator &intercomm)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {

        constexpr auto N = Extents::rank();

        const auto dimensions = get_dimensions(blocks_to_receive);

        // TODO: See if I can omit this by making get_send_receive_package return MultidimensionalBlocks
        const auto multidimensional_blocks_to_send = get_cartesian_product(blocks_to_send);
        const auto multidimensional_blocks_to_receive = get_cartesian_product(blocks_to_receive);

        auto [send_buffer, grouped_blocks_to_send] = get_send_package(
                local_values, multidimensional_blocks_to_send, final_processor_grid);
        auto [receive_buffer, grouped_blocks_to_receive] = get_receive_package<T, N>(
                multidimensional_blocks_to_receive, initial_processor_grid);

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

            auto blocks_from_source =
                    multidimensional_blocks_to_receive |
                    std::views::filter([source, initial_processor_grid](auto block) {
                        return initial_processor_grid.get_processor_id(
                                       get_owner_coordinates(block)) == source;
                    });

            auto received_values =
                    std::span{std::as_const(receive_buffer).begin() + message_starts, num_values};
            for (const auto &block: blocks_from_source) {
                const auto num_values_in_block = get_num_elements(block);

                copy_data(received_values, std::mdspan(new_local_values.data(), dimensions), block);

                received_values = received_values | std::views::drop(num_values_in_block);
            }
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);

        return {new_local_values, dimensions};
    }
}// namespace reshuffle::internal

#endif//MPI_COMM_UTILS_HPP
