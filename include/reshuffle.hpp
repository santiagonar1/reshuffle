#ifndef RESHUFFLE_SHUFFLE_HPP
#define RESHUFFLE_SHUFFLE_HPP

#include <vector>
#include <mpi.h>

namespace reshuffle {

    namespace details {
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

    template<typename T>
    auto shuffle(const std::vector<T> &values, const MPI_Comm &comm) {
        int num_ranks{};
        MPI_Comm_size(comm, &num_ranks);

        int values_per_rank = static_cast<int>(values.size()) / num_ranks;
        MPI_Bcast(&values_per_rank, 1, MPI_INT, 0, comm);

        std::vector<T> my_values(values_per_rank);
        MPI_Scatter(values.data(), values_per_rank, details::to_mpi_datatype<T>(), my_values.data(), my_values.size(),
                    details::to_mpi_datatype<T>(),
                    0, comm);

        return my_values;
    }
}

#endif //RESHUFFLE_SHUFFLE_HPP
