#ifndef RESHUFFLE_RANK_INFORMATION_HPP
#define RESHUFFLE_RANK_INFORMATION_HPP

#include "coordinates.hpp"
#include "inter_communicator.hpp"
#include "mpi_utils.hpp"
#include "processor_grid.hpp"
#include "rank_id.hpp"

namespace reshuffle::internal {
    template<std::size_t N>
    class RankInformation {
    public:
        RankInformation(const InterCommunicator &inter_communicator,
                        const ProcessorGrid<N> &initial_processor_grid,
                        const ProcessorGrid<N> &final_processor_grid);

        [[nodiscard]] auto get_initial_id() const -> RankId;
        [[nodiscard]] auto get_inter_communicator_id() const -> RankId;
        [[nodiscard]] auto get_final_id() const -> RankId;
        [[nodiscard]] auto get_initial_rank_coordinates() const -> Coordinates<N>;
        [[nodiscard]] auto get_final_rank_coordinates() const -> Coordinates<N>;

    private:
        const RankId _initial_id;
        const RankId _inter_communicator_id;
        const RankId _final_id;
        const Coordinates<N> _initial_rank_coordinates;
        const Coordinates<N> _final_rank_coordinates;
    };

    template<std::size_t N>
    RankInformation<N>::RankInformation(const InterCommunicator &inter_communicator,
                                        const ProcessorGrid<N> &initial_processor_grid,
                                        const ProcessorGrid<N> &final_processor_grid)
        : _initial_id(inter_communicator.get_initial_comm_rank().value_or(INVALID_RANK_ID)),
          _inter_communicator_id(
                  mpi::get_rank_id(inter_communicator.get_inter_communicator()).value()),
          _final_id(inter_communicator.get_final_comm_rank().value_or(INVALID_RANK_ID)),
          _initial_rank_coordinates(initial_processor_grid.get_processor_coordinates(_initial_id)),
          _final_rank_coordinates(final_processor_grid.get_processor_coordinates(_final_id)) {}

    template<std::size_t N>
    auto RankInformation<N>::get_initial_id() const -> RankId {
        return _initial_id;
    }

    template<std::size_t N>
    auto RankInformation<N>::get_inter_communicator_id() const -> RankId {
        return _inter_communicator_id;
    }

    template<std::size_t N>
    auto RankInformation<N>::get_final_id() const -> RankId {
        return _final_id;
    }

    template<std::size_t N>
    auto RankInformation<N>::get_initial_rank_coordinates() const -> Coordinates<N> {
        return _initial_rank_coordinates;
    }

    template<std::size_t N>
    auto RankInformation<N>::get_final_rank_coordinates() const -> Coordinates<N> {
        return _final_rank_coordinates;
    }

}// namespace reshuffle::internal

#endif//RESHUFFLE_RANK_INFORMATION_HPP
