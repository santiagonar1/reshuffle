#ifndef RESHUFFLE_MALLEABILITY_SIMULATOR_HPP
#define RESHUFFLE_MALLEABILITY_SIMULATOR_HPP

#include <mpi.h>

#include <expected>
#include <utility>

namespace reshuffle::mpi {

    enum class RequestAdaptationError {
        NOT_ENOUGH_AVAILABLE_RANKS,
    };

    enum class RankStatus {
        JOINING,
        LEAVING,
        STAYING,
        INACTIVE,
    };

    class MalleabilitySimulator {
    public:
        MalleabilitySimulator(const MPI_Comm &base_comm, const MPI_Comm &initial_comm);

        [[nodiscard]] auto request_adaptation(unsigned int num_ranks)
                -> std::expected<std::pair<MPI_Comm &, RankStatus>, RequestAdaptationError>;

    private:
        const MPI_Comm _base_comm;
        MPI_Comm _current_comm;
    };
}// namespace reshuffle::mpi

#endif//RESHUFFLE_MALLEABILITY_SIMULATOR_HPP
