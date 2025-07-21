#ifndef MPI_COMM_UTILS_HPP
#define MPI_COMM_UTILS_HPP

#include <mpi.h>
#include <ranges>

#include "cartesian_product.hpp"
#include "communication_package.hpp"
#include "concepts.hpp"
#include "intercommunicator.hpp"
#include "mpi_utils.hpp"
#include "profiler.hpp"
#include "serialize.hpp"

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
    auto async_send(std::span<T> send_buffer, const std::vector<int> &destinies_final_comm,
                    const Intercommunicator &intercomm) -> std::vector<MPI_Request> {

        if (send_buffer.size() != destinies_final_comm.size()) {
            throw std::invalid_argument("The size of the send buffer and the size of the destinies "
                                        "vector must be equal.");
        }

        const auto num_ranks_to_send = static_cast<int>(destinies_final_comm.size());
        auto send_requests = std::vector<MPI_Request>(num_ranks_to_send);
        const auto comm = intercomm.get_intercommunicator();

        for (int i = 0; i < num_ranks_to_send; i++) {
            constexpr auto num_values = 1;
            const auto rank_id_destiny_final_comm = destinies_final_comm[i];
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

    template<typename T>
    auto async_receive(std::span<T> receive_buffer, const std::vector<int> &sources_initial_comm,
                       const Intercommunicator &intercomm) -> std::vector<MPI_Request> {
        if (receive_buffer.size() != sources_initial_comm.size()) {
            throw std::invalid_argument("The size of the receive buffer and the size of the "
                                        " sources vector must be equal.");
        }
        const auto num_ranks_to_receive = static_cast<int>(sources_initial_comm.size());
        auto receive_requests = std::vector<MPI_Request>(num_ranks_to_receive);
        const auto comm = intercomm.get_intercommunicator();

        for (int i = 0; i < num_ranks_to_receive; i++) {
            constexpr auto num_values = 1;
            // block.get_owner() in this context will return a rank relative to the initial comm.
            // Thus, this should never fail. If it throws, something went wrong.
            const auto rank_id_source_initial_comm = sources_initial_comm[i];
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

    template<concepts::FundamentalType T, typename Extents>
    auto exchange_values(std::mdspan<const T, Extents> local_values,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_send,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_receive,
                         const ProcessorGrid<Extents::rank()> &initial_processor_grid,
                         const ProcessorGrid<Extents::rank()> &final_processor_grid,
                         const Intercommunicator &intercomm)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("exchange_values");

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
            //TODO: Code smell, why [source]? I think this breaks if we don't receive from contiguous ranks
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

    template<typename T>
    auto get_serialized_receive_comm_package(const ReceiveCommunicationPackage<T> &package,
                                             const std::vector<Block> &grouped_blocks_to_send,
                                             const Intercommunicator &intercomm)
            -> ReceiveCommunicationPackage<std::byte> {
        const auto &grouped_blocks_to_receive_after_deserialization = package.data_assignments;

        auto sending_data_to_final_comm = std::vector<int>{};
        auto num_bytes_to_send_per_rank = std::vector<int>{};

        for (const auto &block: grouped_blocks_to_send) {
            sending_data_to_final_comm.push_back(block.get_owner());
            num_bytes_to_send_per_rank.push_back(block.get_interval().get_length());
        }

        auto receiving_data_from_initial_comm = std::vector<int>{};

        for (const auto &block: grouped_blocks_to_receive_after_deserialization) {
            receiving_data_from_initial_comm.push_back(block.get_owner());
        }

        auto num_bytes_to_receive_per_rank =
                std::vector<int>(receiving_data_from_initial_comm.size());

        auto send_size_requests = async_send(std::span{num_bytes_to_send_per_rank},
                                             sending_data_to_final_comm, intercomm);
        auto receive_size_requests = async_receive(std::span{num_bytes_to_receive_per_rank},
                                                   receiving_data_from_initial_comm, intercomm);

        MPI_Waitall(static_cast<int>(receive_size_requests.size()), receive_size_requests.data(),
                    MPI_STATUSES_IGNORE);
        MPI_Waitall(static_cast<int>(send_size_requests.size()), send_size_requests.data(),
                    MPI_STATUSES_IGNORE);

        auto grouped_blocks_to_receive = std::vector<Block>{};

        if (not num_bytes_to_receive_per_rank.empty()) {
            grouped_blocks_to_receive.emplace_back(Block{{0, num_bytes_to_receive_per_rank[0]},
                                                         receiving_data_from_initial_comm[0]});

            for (int i = 1; i < num_bytes_to_receive_per_rank.size(); i++) {
                const auto num_bytes_to_receive = num_bytes_to_receive_per_rank[i];
                const auto rank_id_source_initial_comm = receiving_data_from_initial_comm[i];
                const auto &last_block = grouped_blocks_to_receive.back();
                grouped_blocks_to_receive.emplace_back(
                        Block{{last_block.get_interval().get_right_bound(),
                               last_block.get_interval().get_right_bound() + num_bytes_to_receive},
                              rank_id_source_initial_comm});
            }
        }


        const auto total_num_bytes = std::accumulate(num_bytes_to_receive_per_rank.begin(),
                                                     num_bytes_to_receive_per_rank.end(), 0);

        const auto receive_buffer = std::vector<std::byte>(total_num_bytes);

        return {receive_buffer, grouped_blocks_to_receive};
    }


    template<concepts::NeedsSerialization T, typename Extents>
        requires concepts::Serializable<T>
    auto exchange_values(std::mdspan<const T, Extents> local_values,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_send,
                         const std::array<std::vector<Block>, Extents::rank()> &blocks_to_receive,
                         const ProcessorGrid<Extents::rank()> &initial_processor_grid,
                         const ProcessorGrid<Extents::rank()> &final_processor_grid,
                         const Intercommunicator &intercomm)
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("exchange_values");

        constexpr auto N = Extents::rank();

        const auto dimensions = get_dimensions(blocks_to_receive);

        // TODO: See if I can omit this by making get_send_receive_package return MultidimensionalBlocks
        const auto multidimensional_blocks_to_send = get_cartesian_product(blocks_to_send);
        const auto multidimensional_blocks_to_receive = get_cartesian_product(blocks_to_receive);

        const auto [send_buffer, grouped_blocks_to_send] =
                get_send_package(local_values, multidimensional_blocks_to_send,
                                 final_processor_grid)
                        .as_bytes();

        const auto receive_package = get_receive_package<T, N>(multidimensional_blocks_to_receive,
                                                               initial_processor_grid);

        auto [receive_buffer, grouped_blocks_to_receive] = get_serialized_receive_comm_package(
                receive_package, grouped_blocks_to_send, intercomm);

        auto send_requests = async_send(std::span{send_buffer}, grouped_blocks_to_send, intercomm);
        auto receive_requests =
                async_receive(std::span{receive_buffer}, grouped_blocks_to_receive, intercomm);

        const auto num_ranks_to_receive = static_cast<int>(receive_requests.size());


        auto new_local_values = std::vector<T>(receive_package.buffer.size());
        for (int i = 0; i < num_ranks_to_receive; i++) {
            auto source{MPI_PROC_NULL};

            MPI_Waitany(num_ranks_to_receive, receive_requests.data(), &source, MPI_STATUS_IGNORE);
            // TODO: Fix, might break. Change for a std::map once test passes
            const auto &received_block = grouped_blocks_to_receive[source];
            const auto message_starts = received_block.get_interval().get_left_bound();
            const auto num_values = static_cast<size_t>(received_block.get_interval().get_length());

            auto blocks_from_source =
                    multidimensional_blocks_to_receive |
                    std::views::filter([source, initial_processor_grid](auto block) {
                        return initial_processor_grid.get_processor_id(
                                       get_owner_coordinates(block)) == source;
                    });

            auto received_values_binary =
                    std::span{std::as_const(receive_buffer).begin() + message_starts, num_values};
            const auto received_values = deserialize<T>(received_values_binary);
            auto received_values_view = std::span{received_values};
            for (const auto &block: blocks_from_source) {
                const auto num_values_in_block = get_num_elements(block);

                copy_data(received_values_view, std::mdspan(new_local_values.data(), dimensions),
                          block);

                received_values_view = received_values_view | std::views::drop(num_values_in_block);
            }
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);
        // TODO: Deserialize
        return {new_local_values, dimensions};
    }
}// namespace reshuffle::internal

#endif//MPI_COMM_UTILS_HPP
