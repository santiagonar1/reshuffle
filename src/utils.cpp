#include "utils.hpp"

namespace reshuffle::internal {
    auto have_same_num_values(const BlockCyclic &first, const BlockCyclic &second) -> bool {
        return first.get_num_values() == second.get_num_values();
    }

    auto have_same_num_values(const std::array<BlockCyclic, 2> &first,
                              const std::array<BlockCyclic, 2> &second) -> bool {
        return have_same_num_values(first[0], second[0]) and
               have_same_num_values(first[1], second[1]);
    }
}// namespace reshuffle::internal