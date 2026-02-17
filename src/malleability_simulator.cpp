#include "malleability_simulator.hpp"

#include "mpi_utils.hpp"

#include <cassert>
#include <exception>

namespace reshuffle::mpi {
    namespace internal {
        [[nodiscard]] auto generate_rank_id_vector(unsigned int num_ranks) -> std::vector<RankId> {
            return std::views::iota(0) | std::views::take(num_ranks) |
                   std::ranges::to<std::vector<RankId>>();
        }

        [[nodiscard]] auto get_rank_status(const MPI_Comm &previous_comm, const MPI_Comm &new_comm)
                -> RankStatus {
            if (belongs_to_comm(previous_comm) and belongs_to_comm(new_comm)) {
                return RankStatus::STAYING;
            }

            if (not belongs_to_comm(previous_comm) and belongs_to_comm(new_comm)) {
                return RankStatus::JOINING;
            }

            if (belongs_to_comm(previous_comm) and not belongs_to_comm(new_comm)) {
                return RankStatus::LEAVING;
            }

            if (not belongs_to_comm(previous_comm) and not belongs_to_comm(new_comm)) {
                return RankStatus::INACTIVE;
            }

            throw std::runtime_error("get_rank_status: Unexpected case");
        }
    }// namespace internal

    MalleabilitySimulator::MalleabilitySimulator(const MPI_Comm &base_comm,
                                                 const MPI_Comm &initial_comm)
        : _base_comm{base_comm}, _current_comm{initial_comm} {}

    auto MalleabilitySimulator::request_adaptation(const unsigned int num_ranks)
            -> std::expected<std::pair<MPI_Comm &, RankStatus>, RequestAdaptationError> {
        assert(num_ranks > 0 && "num_ranks must be greater than 0");

        MPI_Barrier(_base_comm);

        if (const auto available_ranks = get_num_ranks(_base_comm); num_ranks > available_ranks) {
            return std::unexpected{RequestAdaptationError::NOT_ENOUGH_AVAILABLE_RANKS};
        }

        const auto new_comm =
                get_sub_comm(_base_comm, internal::generate_rank_id_vector(num_ranks));
        const auto rank_status = internal::get_rank_status(_current_comm, new_comm);

        _current_comm = new_comm;
        return std::pair<MPI_Comm &, RankStatus>{_current_comm, rank_status};
    }

}// namespace reshuffle::mpi