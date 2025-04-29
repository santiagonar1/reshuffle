#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <mpi.h>

#include "block_cyclic.hpp"

namespace reshuffle::dev {
    template<std::size_t N>
    struct Context {
        const BlockCyclic<N> distribution;
        // For some reason, if I do this a reference this segfaults even when
        // called with MPI_COMM_WORLD
        const MPI_Comm comm;

        auto operator==(const Context &other) const -> bool;
    };

    template<std::size_t N>
    auto Context<N>::operator==(const Context &other) const -> bool {
        return distribution == other.distribution and comm == other.comm;
    }
}// namespace reshuffle::dev

#endif//CONTEXT_HPP
