#include "shuffle.hpp"

namespace reshuffle::internal {
    void check_distributions_have_same_num_values(const BlockCyclic &d1, const BlockCyclic &d2) {
        if (not have_same_num_values(d1, d2)) {
            throw std::invalid_argument(
                    "The old and new distributions have different number of values");
        }
    }

    void check_distributions_have_same_num_values(const std::array<BlockCyclic, 2> &d1,
                                                  const std::array<BlockCyclic, 2> &d2) {
        if (not have_same_num_values(d1, d2)) {
            throw std::invalid_argument(
                    "The old and new distributions have different number of values");
        }
    }

    void check_correct_num_values_provided(const BlockCyclic &distribution, const int num_values,
                                           const rank_id rank) {
        if (distribution.get_num_values(rank) != num_values) {
            throw std::invalid_argument(
                    "Number of values provided not consistent with current distribution");
        }
    }

    void check_correct_num_values_provided(const std::array<BlockCyclic, 2> &distribution,
                                           const int num_values, const rank_id rank) {
        if (num_values != num_values_in_rank(distribution, rank)) {
            throw std::invalid_argument(
                    "Number of values provided not consistent with current distribution");
        }
    }

    void check_rank_only_in_destiny_comm_does_not_have_data(const MPI_Comm &origin_comm,
                                                            const MPI_Comm &destiny_comm,
                                                            const bool contains_data) {
        const auto is_rank_only_in_destiny_comm =
                in_mpi_comm(destiny_comm) and not in_mpi_comm(origin_comm);

        if (is_rank_only_in_destiny_comm and contains_data) {
            throw std::invalid_argument("A rank only in destiny communicator contains data");
        }
    }
}// namespace reshuffle::internal
