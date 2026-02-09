#include "processor_info.hpp"

#include <mpi_utils.hpp>

namespace heat {

    ProcessorInfo::ProcessorInfo(const MPI_Comm &cartesian_comm)
        : _neighbours(get_neighbours(cartesian_comm)),
          _rank(reshuffle::mpi::get_rank_id(cartesian_comm).value()) {}

    auto ProcessorInfo::get_rank() const -> int { return _rank; }

    auto ProcessorInfo::get_up_neighbour() const -> int { return _neighbours[UP]; }

    auto ProcessorInfo::get_down_neighbour() const -> int { return _neighbours[DOWN]; }

    auto ProcessorInfo::get_left_neighbour() const -> int { return _neighbours[LEFT]; }

    auto ProcessorInfo::get_right_neighbour() const -> int { return _neighbours[RIGHT]; }

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

    auto ProcessorInfo::get_neighbours(const MPI_Comm &cartesian_comm) -> std::array<int, 4> {
        auto neighbours = std::array<int, 4>{};
        MPI_Cart_shift(cartesian_comm, 1, 1, &neighbours[LEFT], &neighbours[RIGHT]);
        MPI_Cart_shift(cartesian_comm, 0, 1, &neighbours[UP], &neighbours[DOWN]);

        return neighbours;
    }
}// namespace heat