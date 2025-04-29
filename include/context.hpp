#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <mpi.h>

#include "block_cyclic.hpp"

namespace reshuffle::dev {
    struct Context {
        const BlockCyclic<1> distribution;
        // For some reason, if I do this a reference this segfaults even when
        // called with MPI_COMM_WORLD
        const MPI_Comm comm;

        auto operator==(const Context &other) const -> bool;
    };
}// namespace reshuffle::dev

#endif//CONTEXT_HPP
