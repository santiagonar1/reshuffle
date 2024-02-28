#ifndef RESHUFFLE_UTILS_HPP
#define RESHUFFLE_UTILS_HPP

#include <mpi.h>

namespace reshuffle::internal {
    template<typename DATATYPE>
    MPI_Datatype to_mpi_datatype() {
        if (std::is_same_v<DATATYPE, int>) {
            return MPI_INT;
        } else if (std::is_same_v<DATATYPE, float>) {
            return MPI_FLOAT;
        } else if (std::is_same_v<DATATYPE, double>) {
            return MPI_DOUBLE;
        }

        throw std::invalid_argument("No MPI Datatype");
    }
}

#endif //RESHUFFLE_UTILS_HPP
