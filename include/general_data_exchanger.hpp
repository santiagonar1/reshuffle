#ifndef RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP
#define RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP

#include "concepts.hpp"
#include "context.hpp"
#include "data_exchanger.hpp"
#include "dimensions.hpp"
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
        const auto grid_overlay = _initial_context.distribution.get_grid_layout().get_overlay(
                _final_context.distribution.get_grid_layout(),
                _final_context.distribution.get_processor_grid());

        const auto inter_communicator =
                InterCommunicator(_initial_context.comm, _final_context.comm);

        const auto rank_information = RankInformation{
                inter_communicator, _initial_context.distribution.get_processor_grid(),
                _final_context.distribution.get_processor_grid()};

        const auto [blocks_to_send, blocks_to_receive] = get_send_and_receive_blocks(
                grid_overlay, rank_information.get_initial_rank_coordinates(),
                rank_information.get_final_rank_coordinates());

        return internal::exchange_values(_local_values, blocks_to_send, blocks_to_receive,
                                         _initial_context.distribution.get_processor_grid(),
                                         _final_context.distribution.get_processor_grid(),
                                         inter_communicator);
    }
}// namespace reshuffle::internal

#endif//RESHUFFLE_GENERAL_DATA_EXCHANGER_HPP
