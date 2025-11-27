#ifndef INTER_COMMUNICATOR_HPP
#define INTER_COMMUNICATOR_HPP

#include "rank_id.hpp"

#include <map>
#include <mpi.h>
#include <optional>

namespace reshuffle::internal {
    class InterCommunicator {
    public:
        enum class SelectCommunicator {
            INITIAL_COMM,
            FINAL_COMM,
        };

        InterCommunicator(MPI_Comm initial_comm, MPI_Comm final_comm);

        [[nodiscard]] auto get_inter_communicator() const -> MPI_Comm;
        [[nodiscard]] auto get_inter_comm_rank(RankId original_rank,
                                               SelectCommunicator original_comm) const -> RankId;
        [[nodiscard]] auto get_initial_comm_rank(RankId inter_comm_rank) const
                -> std::optional<RankId>;
        [[nodiscard]] auto get_initial_comm_rank() const -> std::optional<RankId>;
        [[nodiscard]] auto get_final_comm_rank(RankId inter_comm_rank) const
                -> std::optional<RankId>;
        [[nodiscard]] auto get_final_comm_rank() const -> std::optional<RankId>;

    private:
        const MPI_Comm _inter_communicator;
        const MPI_Comm _initial_comm;
        const MPI_Comm _final_comm;
        std::map<RankId, RankId> _inter_comm_to_initial_comm;
        std::map<RankId, RankId> _inter_comm_to_final_comm;
        std::map<RankId, RankId> _initial_comm_to_inter_comm;
        std::map<RankId, RankId> _final_comm_to_inter_comm;
    };
}// namespace reshuffle::internal

#endif//INTER_COMMUNICATOR_HPP
