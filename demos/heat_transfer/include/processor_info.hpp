#ifndef RESHUFFLE_PROCESSOR_INFO_HPP
#define RESHUFFLE_PROCESSOR_INFO_HPP

#include "reshuffle.hpp"


#include <array>
#include <expected>

#include <mpi.h>

namespace heat {
    class ProcessorInfo {
    public:
        explicit ProcessorInfo(const MPI_Comm &cartesian_comm);
        ProcessorInfo(reshuffle::RankId rank, reshuffle::RankId up_neighbour,
                      reshuffle::RankId down_neighbour, reshuffle::RankId left_neighbour,
                      reshuffle::RankId right_neighbour);

        [[nodiscard]] auto get_rank() const -> reshuffle::RankId;

        [[nodiscard]] auto get_up_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_down_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_left_neighbour() const -> reshuffle::RankId;
        [[nodiscard]] auto get_right_neighbour() const -> reshuffle::RankId;

        [[nodiscard]] auto has_up_neighbour() const -> bool;
        [[nodiscard]] auto has_down_neighbour() const -> bool;
        [[nodiscard]] auto has_left_neighbour() const -> bool;
        [[nodiscard]] auto has_right_neighbour() const -> bool;

    private:
        enum DIRECTIONS { UP, DOWN, LEFT, RIGHT };
        const std::array<reshuffle::RankId, 4> _neighbours;
        const reshuffle::RankId _rank;

        enum class GetNeighboursError {
            RANK_NOT_IN_COMM,
            COMM_IS_NULL,
        };

        static auto get_neighbours(const MPI_Comm &cartesian_comm)
                -> std::expected<std::array<reshuffle::RankId, 4>, GetNeighboursError>;
        static auto get_neighbours(reshuffle::RankId up_neighbour, reshuffle::RankId down_neighbour,
                                   reshuffle::RankId left_neighbour,
                                   reshuffle::RankId right_neighbour)
                -> std::array<reshuffle::RankId, 4>;
    };
}// namespace heat

#endif//RESHUFFLE_PROCESSOR_INFO_HPP
