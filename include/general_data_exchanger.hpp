#ifndef RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP
#define RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP

#include "concepts.hpp"
#include "context.hpp"
#include "data_exchanger.hpp"
#include "dimensions.hpp"
#include "grid_overlay.hpp"
#include "inter_communicator.hpp"
#include "mdspan.hpp"
#include "mpi_comm_utils.hpp"
#include "rank_information.hpp"

namespace reshuffle::internal {
    template<concepts::Exchangeable T, typename Extents>
    class GeneralDataExchanger final : public DataExchanger<T, Extents::rank()> {
    public:
        GeneralDataExchanger(std::mdspan<const T, Extents> local_values,
                             const Context<Extents::rank()> &initial_context,
                             const Context<Extents::rank()> &final_context);
        ~GeneralDataExchanger() override = default;

        auto exchange() const -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> override;

    private:
        auto
        exchange_impl(const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_send,
                      const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_receive,
                      const InterCommunicator &inter_communicator) const
                -> std::pair<std::vector<T>, Dimensions<Extents::rank()>>;

        std::mdspan<const T, Extents> _local_values;
        const Context<Extents::rank()> _initial_context;
        const Context<Extents::rank()> _final_context;
    };

    template<concepts::Exchangeable T, typename Extents>
    GeneralDataExchanger<T, Extents>::GeneralDataExchanger(
            std::mdspan<const T, Extents> local_values,
            const Context<Extents::rank()> &initial_context,
            const Context<Extents::rank()> &final_context)
        : _local_values(local_values), _initial_context(initial_context),
          _final_context(final_context) {}


    template<concepts::Exchangeable T, typename Extents>
    auto GeneralDataExchanger<T, Extents>::exchange() const
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        const auto grid_overlay = GridOverlay{_initial_context._distribution->get_grid_layout(),
                                              _final_context._distribution->get_grid_layout()};

        const auto inter_communicator =
                InterCommunicator(_initial_context._comm, _final_context._comm);

        const auto this_rank = RankInformation{inter_communicator,
                                               _initial_context._distribution->get_processor_grid(),
                                               _final_context._distribution->get_processor_grid()};

        const auto [blocks_to_send, blocks_to_receive] =
                get_send_and_receive_blocks(grid_overlay, this_rank, IntervalType::LOCAL);

        return exchange_impl(blocks_to_send, blocks_to_receive, inter_communicator);
    }

    template<concepts::Exchangeable T, typename Extents>
    auto GeneralDataExchanger<T, Extents>::exchange_impl(
            const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_send,
            const std::vector<MultidimensionalBlock<Extents::rank()>> &blocks_to_receive,
            const InterCommunicator &inter_communicator) const
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        PROFILE_SCOPE_NAMED("exchange_values");

        constexpr auto N = Extents::rank();
        const auto initial_processor_grid = _initial_context._distribution->get_processor_grid();
        const auto final_processor_grid = _final_context._distribution->get_processor_grid();

        const auto dimensions = get_dimensions(blocks_to_receive);

        auto [send_buffer, intervals_to_send_per_rank] =
                get_send_package(_local_values, blocks_to_send, final_processor_grid);

        const auto inter_comm_rank =
                mpi::get_rank_id(inter_communicator.get_inter_communicator()).value();
        const auto rank_final_comm =
                inter_communicator.get_final_comm_rank().value_or(INVALID_RANK_ID);
        const auto send_to_myself_optional = find(intervals_to_send_per_rank, rank_final_comm);
        intervals_to_send_per_rank.erase(rank_final_comm);

        auto num_messages_to_receive = get_number_of_receive_messages(blocks_to_receive);

        if (send_to_myself_optional.has_value()) { num_messages_to_receive -= 1; }

        auto send_requests =
                async_send(std::span{send_buffer}, intervals_to_send_per_rank, inter_communicator);

        const auto num_new_local_values = get_num_elements(blocks_to_receive);
        auto new_local_values = std::vector<T>(num_new_local_values);

        if (send_to_myself_optional.has_value()) {
            const auto send_to_myself = send_to_myself_optional.value();
            auto blocks_from_source =
                    blocks_to_receive | std::views::filter([inter_comm_rank, initial_processor_grid,
                                                            inter_communicator](auto block) {
                        return initial_processor_grid.get_processor_id(
                                       get_owner_coordinates(block)) ==
                               inter_communicator.get_initial_comm_rank(inter_comm_rank);
                    });

            auto received_values_view =
                    std::span{std::as_const(send_buffer).data() + send_to_myself.get_left_bound(),
                              static_cast<size_t>(send_to_myself.get_length())};
            process_received_blocks(received_values_view,
                                    std::mdspan(new_local_values.data(), dimensions),
                                    blocks_from_source);
        }


        for (int i = 0; i < num_messages_to_receive; i++) {
            const auto [source, data] =
                    mpi::block_receive<T>(inter_communicator.get_inter_communicator());

            auto blocks_from_source =
                    blocks_to_receive | std::views::filter([source, initial_processor_grid,
                                                            inter_communicator](auto block) {
                        return initial_processor_grid.get_processor_id(get_owner_coordinates(
                                       block)) == inter_communicator.get_initial_comm_rank(source);
                    });

            auto received_values_view = std::span{data};
            process_received_blocks(received_values_view,
                                    std::mdspan(new_local_values.data(), dimensions),
                                    blocks_from_source);
        }

        MPI_Waitall(send_requests.size(), send_requests.data(), MPI_STATUSES_IGNORE);

        return {new_local_values, dimensions};
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP
