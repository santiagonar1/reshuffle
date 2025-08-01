#ifndef CONTEXT_CREATION_HPP
#define CONTEXT_CREATION_HPP

#include <shuffle.hpp>

using namespace reshuffle;
using namespace reshuffle::mpi;


[[nodiscard]] auto generate_values(int from, int to) -> std::vector<int>;

class ValuesGenerator {
public:
    ValuesGenerator(const int num_values_per_rank, const int num_ranks)
        : _num_values_per_rank(num_values_per_rank), _num_ranks(num_ranks),
          _values(generate_values(1, num_values_per_rank * num_ranks)) {}

    [[nodiscard]] auto get_values_for_rank(const reshuffle::rank_id rank_id) const
            -> std::vector<int> {
        const auto start = rank_id * _num_values_per_rank;
        const auto end = start + _num_values_per_rank;

        return {_values.begin() + start, _values.begin() + end};
    };

    [[nodiscard]] auto get_all_values() const -> std::vector<int> { return _values; }

    [[nodiscard]] auto get_total_num_values() const -> int {
        return _num_ranks * _num_values_per_rank;
    }

private:
    const int _num_values_per_rank;
    const int _num_ranks;
    const std::vector<int> _values;
};

enum class CommSelector {
    ONLY_RANK_0,
    ONLY_RANK_1,
    ALL_RANKS,
};

enum class DataLocationSelector {
    ONLY_RANK_0,
    ONLY_RANK_1,
    ALL_RANKS,
};

[[nodiscard]] auto create_communicator(const CommSelector &comm_selector) -> MPI_Comm;
[[nodiscard]] auto create_context(const DataLocationSelector &data_location,
                                  const CommSelector &comm_selector, int num_global_values)
        -> Context<1>;
[[nodiscard]] auto create_context(const DataLocationSelector &data_location, MPI_Comm comm,
                                  int num_global_values) -> Context<1>;
[[nodiscard]] auto is_disjoint(const DataLocationSelector &data_location,
                               const CommSelector &comm_selector) -> bool;
[[nodiscard]] auto is_rank_with_data_outside_comm(const DataLocationSelector &data_location,
                                                  const CommSelector &comm_selector) -> bool;

inline auto generate_values(int from, int to) -> std::vector<int> {
    auto values_range = std::views::iota(from, to + 1);
    return {values_range.begin(), values_range.end()};
}

inline auto create_communicator(const CommSelector &comm_selector) -> MPI_Comm {
    switch (comm_selector) {
        case CommSelector::ONLY_RANK_0:
            return get_sub_comm(MPI_COMM_WORLD, std::vector(1, 0));
        case CommSelector::ONLY_RANK_1:
            return get_sub_comm(MPI_COMM_WORLD, std::vector(1, 1));
        case CommSelector::ALL_RANKS:
            return MPI_COMM_WORLD;
        default:
            throw std::runtime_error("Invalid CommSelector");
    }
}

inline auto create_context(const DataLocationSelector &data_location,
                           const CommSelector &comm_selector, const int num_global_values)
        -> Context<1> {
    if (is_rank_with_data_outside_comm(data_location, comm_selector)) {
        throw std::runtime_error("Want to allocate data in rank outside of communicator");
    }

    const auto comm = create_communicator(comm_selector);
    return create_context(data_location, comm, num_global_values);
}

inline auto create_context(const DataLocationSelector &data_location, const MPI_Comm comm,
                           const int num_global_values) -> Context<1> {
    switch (data_location) {
        case DataLocationSelector::ONLY_RANK_0:
        case DataLocationSelector::ONLY_RANK_1:
            return Context{make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{1}}),
                           comm};
        case DataLocationSelector::ALL_RANKS:
            return Context{make_block_wise_distribution({num_global_values}, ProcessorGrid<1>{{2}}),
                           comm};
        default:
            throw std::runtime_error("Invalid DataLocationSelector");
    }
}

inline auto is_disjoint(const DataLocationSelector &data_location,
                        const CommSelector &comm_selector) -> bool {
    const auto disjoint = (comm_selector == CommSelector::ONLY_RANK_0 and
                           data_location == DataLocationSelector::ONLY_RANK_1) or
                          (comm_selector == CommSelector::ONLY_RANK_1 and
                           data_location == DataLocationSelector::ONLY_RANK_0);
    return disjoint;
}

inline auto is_rank_with_data_outside_comm(const DataLocationSelector &data_location,
                                    const CommSelector &comm_selector) -> bool {
    const auto data_outside_boundaries_comm = (comm_selector == CommSelector::ONLY_RANK_0 or
                                               comm_selector == CommSelector::ONLY_RANK_1) and
                                              data_location == DataLocationSelector::ALL_RANKS;
    return is_disjoint(data_location, comm_selector) or data_outside_boundaries_comm;
}

#endif//CONTEXT_CREATION_HPP
