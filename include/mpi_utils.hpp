#ifndef RESHUFFLE_MPI_UTILS_HPP
#define RESHUFFLE_MPI_UTILS_HPP

#include <algorithm>
#include <mpi.h>
#include <vector>

#include "rank_id.hpp"

namespace reshuffle::mpi {
    template<typename DATATYPE>
    [[nodiscard]] MPI_Datatype to_mpi_datatype() {
        if (std::is_same_v<DATATYPE, int>) { return MPI_INT; }

        if (std::is_same_v<DATATYPE, float>) { return MPI_FLOAT; }

        if (std::is_same_v<DATATYPE, double>) { return MPI_DOUBLE; }

        if (std::is_same_v<DATATYPE, std::byte>) { return MPI_BYTE; }

        throw std::invalid_argument("No MPI Datatype");
    }

    [[nodiscard]] auto get_rank_id(const MPI_Comm &comm) -> rank_id;

    [[nodiscard]] auto is_root(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_num_ranks(const MPI_Comm &comm) -> int;

    [[nodiscard]] auto is_comm_null(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto get_sub_comm(MPI_Comm base_comm, const std::vector<rank_id> &ranks)
            -> MPI_Comm;

    [[nodiscard]] auto get_group(MPI_Comm comm) -> std::optional<MPI_Group>;

    [[nodiscard]] auto belongs_to_comm(const MPI_Comm &comm) -> bool;

    [[nodiscard]] auto is_sub_comm(MPI_Comm comm, MPI_Comm possible_sub_comm) -> bool;

    [[nodiscard]] auto get_contiguous_datatype(MPI_Datatype base_datatype,
                                               int num_consecutive_elements) -> MPI_Datatype;
}// namespace reshuffle::mpi

#endif//RESHUFFLE_MPI_UTILS_HPP
