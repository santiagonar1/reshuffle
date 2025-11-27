#ifndef INTERCOMMUNICATOR_HPP
#define INTERCOMMUNICATOR_HPP

#include "rank_id.hpp"

#include <map>
#include <mpi.h>
#include <optional>

namespace reshuffle::internal {
    class Intercommunicator {
    public:
        enum class SelectCommunicator {
            INITIAL_COMM,
            FINAL_COMM,
        };

        Intercommunicator(MPI_Comm initial_comm, MPI_Comm final_comm);

        [[nodiscard]] auto get_intercommunicator() const -> MPI_Comm;
        [[nodiscard]] auto get_intercomm_rank(RankId original_rank,
                                              SelectCommunicator original_comm) const -> RankId;
        [[nodiscard]] auto get_initial_comm_rank(RankId intercomm_rank) const
                -> std::optional<RankId>;
        [[nodiscard]] auto get_initial_comm_rank() const -> std::optional<RankId>;
        [[nodiscard]] auto get_final_comm_rank(RankId intercomm_rank) const
                -> std::optional<RankId>;
        [[nodiscard]] auto get_final_comm_rank() const -> std::optional<RankId>;

    private:
        const MPI_Comm _intercommunicator;
        const MPI_Comm _initial_comm;
        const MPI_Comm _final_comm;
        std::map<RankId, RankId> _intercomm_to_initial_comm;
        std::map<RankId, RankId> _intercomm_to_final_comm;
        std::map<RankId, RankId> _initial_comm_to_intercomm;
        std::map<RankId, RankId> _final_comm_to_intercomm;
    };
}// namespace reshuffle::internal

#endif//INTERCOMMUNICATOR_HPP
