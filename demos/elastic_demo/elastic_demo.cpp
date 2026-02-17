#include <iostream>
#include <mpi.h>
#include <vector>

#include <block_wise.hpp>
#include <reshuffle.hpp>


auto is_rank_active(int num_active_ranks) -> bool;
auto simulate_adaptation(int num_active_ranks) -> MPI_Comm;

auto operator<<(std::ostream &os, const std::vector<int> &values) -> std::ostream &;

int main() {
    constexpr auto num_values_per_rank = 10;

    MPI_Init(nullptr, nullptr);

    const auto available_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);
    const auto rank = reshuffle::mpi::get_rank_id(MPI_COMM_WORLD).value();

    const auto original_buffer = std::vector(num_values_per_rank, rank);
    const auto global_num_values = num_values_per_rank * available_ranks;
    const auto global_dimensions = reshuffle::Dimensions{global_num_values};

    const auto origin_context = reshuffle::Context{
            reshuffle::distribution::BlockWise{global_dimensions,
                                               reshuffle::ProcessorGrid{available_ranks}},
            MPI_COMM_WORLD};
    for (int active_ranks = 1; active_ranks <= available_ranks; active_ranks++) {
        auto comm = simulate_adaptation(active_ranks);
        const auto destiny_context = reshuffle::Context{
                reshuffle::distribution::BlockWise{global_dimensions,
                                                   reshuffle::ProcessorGrid{active_ranks}},
                comm};
        const auto buffer =
                reshuffle::shuffle(std::mdspan{original_buffer.data(), original_buffer.size()},
                                   origin_context, destiny_context)
                        .first;

        if (is_rank_active(active_ranks)) {
            if (reshuffle::mpi::is_root(comm)) { std::cout << buffer << std::endl; }
            MPI_Comm_free(&comm);
        }
    }


    MPI_Finalize();
    return 0;
}

auto simulate_adaptation(const int num_active_ranks) -> MPI_Comm {
    MPI_Barrier(MPI_COMM_WORLD);

    const auto active_ranks = std::views::iota(0, num_active_ranks) |
                              std::ranges::to<std::vector<reshuffle::RankId>>();

    return reshuffle::mpi::get_sub_comm(MPI_COMM_WORLD, active_ranks);
}

auto is_rank_active(const int num_active_ranks) -> bool {
    const auto rank = reshuffle::mpi::get_rank_id(MPI_COMM_WORLD).value();
    return rank < num_active_ranks;
}

auto operator<<(std::ostream &os, const std::vector<int> &values) -> std::ostream & {
    os << "[";
    auto delimiter = std::string{};
    for (const auto value: values) { os << std::exchange(delimiter, ", ") << value; }
    return os << "]";
}