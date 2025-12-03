#ifndef RESHUFFLE_SCATTER_EXCHANGER_HPP
#define RESHUFFLE_SCATTER_EXCHANGER_HPP

#include "data_exchanger.hpp"
#include "grid_overlay.hpp"

namespace reshuffle::internal {
    template<concepts::Exchangeable T, typename Extents>
    class ScatterExchanger final : public DataExchanger<T, Extents::rank()> {
    public:
        ScatterExchanger(std::mdspan<const T, Extents> local_values,
                         const Context<Extents::rank()> &initial_context,
                         const Context<Extents::rank()> &final_context);
        ~ScatterExchanger() override = default;

        auto exchange() const -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> override;

    private:
        std::mdspan<const T, Extents> _local_values;
        const Context<Extents::rank()> _initial_context;
        const Context<Extents::rank()> _final_context;
    };

    template<concepts::Exchangeable T, typename Extents>
    ScatterExchanger<T, Extents>::ScatterExchanger(std::mdspan<const T, Extents> local_values,
                                                   const Context<Extents::rank()> &initial_context,
                                                   const Context<Extents::rank()> &final_context)
        : _local_values(local_values), _initial_context(initial_context),
          _final_context(final_context) {}

    template<concepts::Exchangeable T, typename Extents>
    auto ScatterExchanger<T, Extents>::exchange() const
            -> std::pair<std::vector<T>, Dimensions<Extents::rank()>> {
        const auto grid_overlay = GridOverlay{_initial_context.distribution.get_grid_layout(),
                                              _final_context.distribution.get_grid_layout()};

        const auto inter_communicator =
                InterCommunicator(_initial_context.comm, _final_context.comm);

        const auto rank_information = RankInformation{
                inter_communicator, _initial_context.distribution.get_processor_grid(),
                _final_context.distribution.get_processor_grid()};

        const auto [blocks_to_send, blocks_to_receive] = get_send_and_receive_blocks(
                grid_overlay, rank_information.get_initial_rank_coordinates(),
                rank_information.get_final_rank_coordinates());

        const auto multidimensional_blocks = grid_overlay.get_multidimensional_blocks_origin();

        const auto root_coordinates = get_owner_coordinates(multidimensional_blocks[0]);

        const auto root_rank_initial_comm =
                _initial_context.distribution.get_processor_grid().get_processor_id(
                        root_coordinates);
        const auto root_inter_comm = inter_communicator.get_inter_comm_rank(
                root_rank_initial_comm, InterCommunicator::SelectCommunicator::INITIAL_COMM);
        return internal::scatter_values(_local_values, blocks_to_send, blocks_to_receive,
                                        _final_context.distribution.get_processor_grid(),
                                        root_inter_comm, inter_communicator);
    }


}// namespace reshuffle::internal

#endif//RESHUFFLE_SCATTER_EXCHANGER_HPP
