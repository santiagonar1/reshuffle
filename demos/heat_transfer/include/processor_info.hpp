#ifndef RESHUFFLE_PROCESSOR_INFO_HPP
#define RESHUFFLE_PROCESSOR_INFO_HPP

#include "reshuffle.hpp"


#include <array>
#include <mpi.h>

namespace heat {
    class ProcessorInfo {
    public:
        explicit ProcessorInfo(const MPI_Comm &cartesian_comm);

        [[nodiscard]] auto get_rank() const -> int;

        [[nodiscard]] auto get_up_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_down_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_left_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_right_neighbour() const -> reshuffle::RankId;

        [[nodiscard]] auto has_up_neighbour() const -> bool;
        [[nodiscard]] auto has_down_neighbour() const -> bool;
        [[nodiscard]] auto has_left_neighbour() const -> bool;
        [[nodiscard]] auto has_right_neighbour() const -> bool;

    private:
        enum DIRECTIONS { DOWN, UP, LEFT, RIGHT };
        const std::array<int, 4> _neighbours;
        const int _rank;

        static auto get_neighbours(const MPI_Comm &cartesian_comm) -> std::array<int, 4>;
    };
}// namespace heat

#endif//RESHUFFLE_PROCESSOR_INFO_HPP
