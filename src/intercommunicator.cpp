#include "intercommunicator.hpp"

#include "contiguous_mpi_datatype.hpp"
#include "mpi_utils.hpp"

#include <array>
#include <stdexcept>

namespace reshuffle::internal {
    auto create_intercommunicator(MPI_Comm comm1, MPI_Comm comm2) -> MPI_Comm {
        if (mpi::is_sub_comm(comm1, comm2)) { return comm1; }
        if (mpi::is_sub_comm(comm2, comm1)) { return comm2; }
        throw std::runtime_error("intercommunicator requires for now one of the communicators "
                                 " to be a sub_communicator");
    }

    //Intercommunicator enables communication between ranks with different MPI_Comm
    Intercommunicator::Intercommunicator(const MPI_Comm initial_comm, const MPI_Comm final_comm)
        : _intercommunicator{create_intercommunicator(initial_comm, final_comm)},
          _initial_comm{initial_comm}, _final_comm{final_comm} {
        //gets the rank in the MPI_Comm that is NOT the sub communicator
        const auto rank_intercomm = mpi::get_rank_id(_intercommunicator).value();

        auto rank_first_comm{INVALID_RANK_ID};
        auto rank_second_comm{INVALID_RANK_ID};

        if (mpi::belongs_to_comm(initial_comm)) {
            rank_first_comm = mpi::get_rank_id(initial_comm).value();
        }

        if (mpi::belongs_to_comm(final_comm)) {
            rank_second_comm = mpi::get_rank_id(final_comm).value();
        }

        const auto info_rank = std::array{rank_first_comm, rank_second_comm, rank_intercomm};
        const auto num_ranks = mpi::get_num_ranks(_intercommunicator);

        const auto info_rank_datatype = mpi::ContiguousMPIDatatype{MPI_INT, info_rank.size()};
        auto info_all_ranks = std::vector<std::array<int, 3>>(num_ranks);

        MPI_Allgather(info_rank.data(), 1, info_rank_datatype.get_datatype(), info_all_ranks.data(),
                      1, info_rank_datatype.get_datatype(), _intercommunicator);

        for (const auto &[rank_first_comm, rank_second_comm, rank_intercomm]: info_all_ranks) {
            if (rank_first_comm != INVALID_RANK_ID) {
                _intercomm_to_initial_comm[rank_intercomm] = rank_first_comm;
                _initial_comm_to_intercomm[rank_first_comm] = rank_intercomm;
            }

            if (rank_second_comm != INVALID_RANK_ID) {
                _intercomm_to_final_comm[rank_intercomm] = rank_second_comm;
                _final_comm_to_intercomm[rank_second_comm] = rank_intercomm;
            }
        }
    }

    auto Intercommunicator::get_intercommunicator() const -> MPI_Comm { return _intercommunicator; }

    auto Intercommunicator::get_intercomm_rank(const RankId original_rank,
                                               const SelectCommunicator original_comm) const
            -> RankId {
        switch (original_comm) {
            case SelectCommunicator::INITIAL_COMM:
                if (_initial_comm_to_intercomm.contains(original_rank)) {
                    return _initial_comm_to_intercomm.at(original_rank);
                }
                throw std::invalid_argument("Rank does not belong to the initial communicator");
            case SelectCommunicator::FINAL_COMM:
                if (_final_comm_to_intercomm.contains(original_rank)) {
                    return _final_comm_to_intercomm.at(original_rank);
                }
                throw std::invalid_argument("Rank does not belong to the final communicator");
        }

        throw std::runtime_error("It should never reach here");
    }

    auto Intercommunicator::get_initial_comm_rank(RankId intercomm_rank) const
            -> std::optional<RankId> {
        if (_intercomm_to_initial_comm.contains(intercomm_rank)) {
            return _intercomm_to_initial_comm.at(intercomm_rank);
        }

        return std::nullopt;
    }
    auto Intercommunicator::get_final_comm_rank(RankId intercomm_rank) const
            -> std::optional<RankId> {
        if (_intercomm_to_final_comm.contains(intercomm_rank)) {
            return _intercomm_to_final_comm.at(intercomm_rank);
        }

        return std::nullopt;
    }

}// namespace reshuffle::internal
