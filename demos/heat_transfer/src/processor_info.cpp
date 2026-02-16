#include "processor_info.hpp"

#include <mpi_utils.hpp>

namespace heat {
    ProcessorInfo::ProcessorInfo(const MPI_Comm &cartesian_comm)
        : _neighbours(get_neighbours(cartesian_comm)
                              .value_or(std::array{MPI_PROC_NULL, MPI_PROC_NULL, MPI_PROC_NULL,
                                                   MPI_PROC_NULL})),
          _rank(reshuffle::mpi::get_rank_id(cartesian_comm).value_or(MPI_PROC_NULL)) {}

    ProcessorInfo::ProcessorInfo(const reshuffle::RankId rank, const reshuffle::RankId up_neighbour,
                                 const reshuffle::RankId down_neighbour,
                                 const reshuffle::RankId left_neighbour,
                                 const reshuffle::RankId right_neighbour)
        : _neighbours{get_neighbours(up_neighbour, down_neighbour, left_neighbour,
                                     right_neighbour)},
          _rank(rank) {}

    auto ProcessorInfo::get_rank() const -> int { return _rank; }

    auto ProcessorInfo::get_up_neighbour() const -> reshuffle::RankId { return _neighbours[UP]; }

    auto ProcessorInfo::get_down_neighbour() const -> reshuffle::RankId {
        return _neighbours[DOWN];
    }

    auto ProcessorInfo::get_left_neighbour() const -> reshuffle::RankId {
        return _neighbours[LEFT];
    }

    auto ProcessorInfo::get_right_neighbour() const -> reshuffle::RankId {
        return _neighbours[RIGHT];
    }

    auto ProcessorInfo::has_up_neighbour() const -> bool {
        return _neighbours[UP] != MPI_PROC_NULL;
    }

    auto ProcessorInfo::has_down_neighbour() const -> bool {
        return _neighbours[DOWN] != MPI_PROC_NULL;
    }

    auto ProcessorInfo::has_left_neighbour() const -> bool {
        return _neighbours[LEFT] != MPI_PROC_NULL;
    }

    auto ProcessorInfo::has_right_neighbour() const -> bool {
        return _neighbours[RIGHT] != MPI_PROC_NULL;
    }

    auto ProcessorInfo::get_neighbours(const MPI_Comm &cartesian_comm)
            -> std::expected<std::array<reshuffle::RankId, 4>, GetNeighboursError> {

        if (cartesian_comm == MPI_COMM_NULL) {
            return std::unexpected{GetNeighboursError::COMM_IS_NULL};
        }

        if (not reshuffle::mpi::belongs_to_comm(cartesian_comm)) {
            return std::unexpected{GetNeighboursError::COMM_IS_NULL};
        }

        auto neighbours = std::array<reshuffle::RankId, 4>{};
        MPI_Cart_shift(cartesian_comm, 1, 1, &neighbours[LEFT], &neighbours[RIGHT]);
        MPI_Cart_shift(cartesian_comm, 0, 1, &neighbours[UP], &neighbours[DOWN]);

        return neighbours;
    }

    auto ProcessorInfo::get_neighbours(const reshuffle::RankId up_neighbour,
                                       const reshuffle::RankId down_neighbour,
                                       const reshuffle::RankId left_neighbour,
                                       const reshuffle::RankId right_neighbour)
            -> std::array<reshuffle::RankId, 4> {
        auto neighbours = std::array<reshuffle::RankId, 4>{};

        neighbours[UP] = up_neighbour;
        neighbours[DOWN] = down_neighbour;
        neighbours[LEFT] = left_neighbour;
        neighbours[RIGHT] = right_neighbour;

        return neighbours;
    }
}// namespace heat