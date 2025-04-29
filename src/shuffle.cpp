#include "shuffle.hpp"

namespace reshuffle::dev::internal {
    // TODO: change to accept multiple dimensions, for now assuming only one
    auto get_send_and_receive_blocks(const GridOverlay<1> &grid_overlay, const rank_id initial_rank,
                                     const rank_id final_rank)
            -> std::pair<std::vector<Block>, std::vector<Block>> {
        auto send_blocks = std::vector<Block>{};
        auto receive_blocks = std::vector<Block>{};

        // TODO: change to accept multiple dimensions, for now assuming only one
        const auto blocks = grid_overlay.grid.get_blocks().at(0);
        for (int i = 0; i < blocks.size(); ++i) {
            const auto &block = blocks[i];
            if (block.get_owner() == initial_rank) {
                send_blocks.emplace_back(block.get_interval(),
                                         grid_overlay.owners_target_grid[0][i]);
            }

            if (grid_overlay.owners_target_grid[0][i] == final_rank) {
                receive_blocks.emplace_back(block);
            }
        }

        send_blocks = join(send_blocks);
        receive_blocks = join(receive_blocks);

        return {send_blocks, receive_blocks};
    }
}// namespace reshuffle::dev::internal

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
        const auto expected = distribution.get_num_values_hold_by(rank);

        if (distribution.get_num_values_hold_by(rank) != num_values) {
            const std::string error_msg = "Number of values provided not consistent with current "
                                          "distribution. Expected: " +
                                          std::to_string(expected) +
                                          " but got: " + std::to_string(num_values);
            throw std::invalid_argument(error_msg);
        }
    }

    void check_correct_num_values_provided(const std::array<BlockCyclic, 2> &distribution,
                                           const int num_values, const rank_id rank) {
        const auto expected = num_values_in_rank(distribution, rank);

        if (num_values != expected) {
            const std::string error_msg = "Number of values provided not consistent with current "
                                          "distribution. Expected: " +
                                          std::to_string(expected) +
                                          " but got: " + std::to_string(num_values);
            throw std::invalid_argument(error_msg);
        }
    }

    void check_rank_only_in_destiny_comm_does_not_have_data(const MPI_Comm &origin_comm,
                                                            const MPI_Comm &destiny_comm,
                                                            const bool contains_data) {
        const auto is_rank_only_in_destiny_comm =
                mpi::in_mpi_comm(destiny_comm) and not mpi::in_mpi_comm(origin_comm);

        if (is_rank_only_in_destiny_comm and contains_data) {
            throw std::invalid_argument("A rank only in destiny communicator contains data");
        }
    }
}// namespace reshuffle::internal
