#include "inter_communicator.hpp"

#include "contiguous_mpi_datatype.hpp"
#include "mpi_utils.hpp"

#include <array>
#include <stdexcept>

namespace reshuffle::internal {
    auto create_inter_communicator(MPI_Comm comm1, MPI_Comm comm2) -> MPI_Comm {
        if (not mpi::belongs_to_comm(comm1) and not mpi::belongs_to_comm(comm2)) {
            return MPI_COMM_NULL;
        }

        if (mpi::is_sub_comm(comm1, comm2)) { return comm1; }
        if (mpi::is_sub_comm(comm2, comm1)) { return comm2; }
        throw std::runtime_error("inter-communicator requires for now one of the communicators "
                                 " to be a sub_communicator");
    }

    // TODO: Deal with partially disjoint communicators
    //InterCommunicator enables communication between ranks with different MPI_Comm
    InterCommunicator::InterCommunicator(const MPI_Comm initial_comm, const MPI_Comm final_comm)
        : _inter_communicator{create_inter_communicator(initial_comm, final_comm)},
          _initial_comm{initial_comm}, _final_comm{final_comm} {
        // If this is called from a rank in neither of the communicators, skip initialization
        if (not mpi::belongs_to_comm(_inter_communicator)) { return; }

        //gets the rank in the MPI_Comm that is NOT the sub communicator
        const auto rank_inter_comm = mpi::get_rank_id(_inter_communicator).value();

        auto rank_first_comm{INVALID_RANK_ID};
        auto rank_second_comm{INVALID_RANK_ID};

        if (mpi::belongs_to_comm(initial_comm)) {
            rank_first_comm = mpi::get_rank_id(initial_comm).value();
        }

        if (mpi::belongs_to_comm(final_comm)) {
            rank_second_comm = mpi::get_rank_id(final_comm).value();
        }

        const auto info_rank = std::array{rank_first_comm, rank_second_comm, rank_inter_comm};
        const auto num_ranks = mpi::get_num_ranks(_inter_communicator);

        const auto info_rank_datatype = mpi::ContiguousMPIDatatype{MPI_INT, info_rank.size()};
        auto info_all_ranks = std::vector<std::array<int, 3>>(num_ranks);

        MPI_Allgather(info_rank.data(), 1, info_rank_datatype.get_datatype(), info_all_ranks.data(),
                      1, info_rank_datatype.get_datatype(), _inter_communicator);

        for (const auto &[rank_first_comm, rank_second_comm, rank_inter_comm]: info_all_ranks) {
            if (rank_first_comm != INVALID_RANK_ID) {
                _inter_comm_to_initial_comm[rank_inter_comm] = rank_first_comm;
                _initial_comm_to_inter_comm[rank_first_comm] = rank_inter_comm;
            }

            if (rank_second_comm != INVALID_RANK_ID) {
                _inter_comm_to_final_comm[rank_inter_comm] = rank_second_comm;
                _final_comm_to_inter_comm[rank_second_comm] = rank_inter_comm;
            }
        }
    }

    auto InterCommunicator::get_inter_communicator() const -> MPI_Comm {
        return _inter_communicator;
    }

    auto InterCommunicator::get_inter_comm_rank(const RankId original_rank,
                                                const SelectCommunicator original_comm) const
            -> RankId {
        switch (original_comm) {
            case SelectCommunicator::INITIAL_COMM:
                if (_initial_comm_to_inter_comm.contains(original_rank)) {
                    return _initial_comm_to_inter_comm.at(original_rank);
                }
                throw std::invalid_argument("Rank does not belong to the initial communicator");
            case SelectCommunicator::FINAL_COMM:
                if (_final_comm_to_inter_comm.contains(original_rank)) {
                    return _final_comm_to_inter_comm.at(original_rank);
                }
                throw std::invalid_argument("Rank does not belong to the final communicator");
        }

        throw std::runtime_error("It should never reach here");
    }

    auto InterCommunicator::get_initial_comm_rank(RankId inter_comm_rank) const
            -> std::optional<RankId> {
        if (_inter_comm_to_initial_comm.contains(inter_comm_rank)) {
            return _inter_comm_to_initial_comm.at(inter_comm_rank);
        }

        return std::nullopt;
    }

    auto InterCommunicator::get_initial_comm_rank() const -> std::optional<RankId> {
        const auto inter_comm_rank_opt = mpi::get_rank_id(_inter_communicator);

        if (not inter_comm_rank_opt.has_value()) { return std::nullopt; }

        if (const auto inter_comm_rank = inter_comm_rank_opt.value();
            _inter_comm_to_initial_comm.contains(inter_comm_rank)) {
            return _inter_comm_to_initial_comm.at(inter_comm_rank);
        }

        return std::nullopt;
    }

    auto InterCommunicator::get_final_comm_rank(RankId inter_comm_rank) const
            -> std::optional<RankId> {
        if (_inter_comm_to_final_comm.contains(inter_comm_rank)) {
            return _inter_comm_to_final_comm.at(inter_comm_rank);
        }

        return std::nullopt;
    }

    auto InterCommunicator::get_final_comm_rank() const -> std::optional<RankId> {
        const auto inter_comm_rank_opt = mpi::get_rank_id(_inter_communicator);

        if (not inter_comm_rank_opt.has_value()) { return std::nullopt; }

        if (const auto inter_comm_rank = inter_comm_rank_opt.value();
            _inter_comm_to_final_comm.contains(inter_comm_rank)) {
            return _inter_comm_to_final_comm.at(inter_comm_rank);
        }

        return std::nullopt;
    }

}// namespace reshuffle::internal
