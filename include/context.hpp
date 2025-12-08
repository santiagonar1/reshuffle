#ifndef CONTEXT_HPP
#define CONTEXT_HPP

#include <mpi.h>

#include "data_distribution.hpp"

namespace reshuffle {
    template<std::size_t N>
    class Context {
    public:
        Context(const DataDistribution<N> &distribution, const MPI_Comm &comm);

        // Move
        Context(Context &&other) noexcept = default;
        Context &operator=(Context &&other) noexcept = default;

        // Copy
        Context(const Context &other);
        Context &operator=(const Context &other);

        [[nodiscard]] auto get_comm() const -> MPI_Comm;
        [[nodiscard]] auto get_distribution() const -> const DataDistribution<N> &;

        auto operator==(const Context &other) const -> bool;

    private:
        const std::unique_ptr<const DataDistribution<N>> _distribution;
        // For some reason, if I do this a reference this segfaults even when
        // called with MPI_COMM_WORLD
        MPI_Comm _comm;
    };

    template<std::size_t N>
    Context<N>::Context(const DataDistribution<N> &distribution, const MPI_Comm &comm)
        : _distribution(distribution.clone()), _comm(comm) {}

    template<std::size_t N>
    auto Context<N>::operator==(const Context &other) const -> bool {
        return _distribution == other._distribution and _comm == other._comm;
    }

    template<std::size_t N>
    Context<N>::Context(const Context &other)
        : _distribution(other._distribution ? other._distribution->clone() : nullptr),
          _comm(other._comm) {}

    template<std::size_t N>
    auto Context<N>::get_comm() const -> MPI_Comm {
        return _comm;
    }

    template<std::size_t N>
    auto Context<N>::get_distribution() const -> const DataDistribution<N> & {
        return *_distribution;
    }

    template<std::size_t N>
    Context<N> &Context<N>::operator=(const Context &other) {
        _distribution = other._distribution ? other._distribution->clone() : nullptr;
        _comm = other._comm;
        return *this;
    }
}// namespace reshuffle

#endif//CONTEXT_HPP
