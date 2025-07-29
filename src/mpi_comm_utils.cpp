#include "mpi_comm_utils.hpp"

namespace reshuffle::internal {
    auto get_num_elements(const MPI_Status &status, const MPI_Datatype &datatype) -> int {
        int count{};
        MPI_Get_count(&status, datatype, &count);
        return count;
    }

    auto block_until_message_is_received(const MPI_Datatype &datatype, const MPI_Comm &comm)
            -> ReceivedMessageInformation {
        MPI_Status status{};
        MPI_Probe(MPI_ANY_SOURCE, MPI_ANY_TAG, comm, &status);
        return {status.MPI_SOURCE, status.MPI_TAG, get_num_elements(status, datatype)};
    }
}// namespace reshuffle::internal::test