#include <iostream>
#include <mpi.h>
#include <vector>

#include <block_wise.hpp>
#include <reshuffle.hpp>


[[nodiscard]] auto simulate_adaptation(int num_active_ranks) -> MPI_Comm;

auto print_domain_decomposition(const reshuffle::Context<1> &current_context,
                                const reshuffle::Dimensions<1> &local_dimensions,
                                const reshuffle::Dimensions<1> &global_dimensions) -> void;

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

    const auto all_in_root = reshuffle::Context{
            reshuffle::distribution::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1}},
            MPI_COMM_WORLD};

    const auto global_buffer =
            reshuffle::shuffle(std::mdspan{original_buffer.data(), original_buffer.size()},
                               origin_context, all_in_root)
                    .first;

    if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
        std::cout << "\n\n*******************************************************\n\n";

        std::cout << "Initial values" << std::endl;
        std::cout << global_buffer << std::endl;
    }

    for (int active_ranks = 1; active_ranks <= available_ranks; active_ranks++) {
        auto comm = simulate_adaptation(active_ranks);
        const auto destiny_context = reshuffle::Context{
                reshuffle::distribution::BlockWise{global_dimensions,
                                                   reshuffle::ProcessorGrid{active_ranks}},
                comm};
        const auto [buffer, local_dimensions] =
                reshuffle::shuffle(std::mdspan{original_buffer.data(), original_buffer.size()},
                                   origin_context, destiny_context);

        if (reshuffle::mpi::belongs_to_comm(comm)) {
            if (reshuffle::mpi::is_root(comm)) {
                std::cout << "\n\n******** Active Ranks " << active_ranks << " *************\n\n";
                std::cout << "Values in Rank 0 " << std::endl;
                std::cout << buffer << std::endl << std::endl;
            }
            print_domain_decomposition(destiny_context, local_dimensions, global_dimensions);
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

auto print_domain_decomposition(const reshuffle::Context<1> &current_context,
                                const reshuffle::Dimensions<1> &local_dimensions,
                                const reshuffle::Dimensions<1> &global_dimensions) -> void {
    const auto comm = current_context.get_comm();
    const auto rank = reshuffle::mpi::get_rank_id(comm).value_or(MPI_PROC_NULL);


    const auto local_decomposition = std::vector(local_dimensions[0], rank);

    const auto all_in_root = reshuffle::Context{
            reshuffle::distribution::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1}},
            comm};

    const auto [global_decomposition_values, dimensions] =
            reshuffle::shuffle(std::mdspan{local_decomposition.data(), local_dimensions[0]},
                               current_context, all_in_root);

    if (reshuffle::mpi::is_root(comm)) {
        std::println("Domain Decomposition");
        std::cout << global_decomposition_values << std::endl;
    }
}

auto operator<<(std::ostream &os, const std::vector<int> &values) -> std::ostream & {
    os << "[";
    auto delimiter = std::string{};
    for (const auto value: values) { os << std::exchange(delimiter, ", ") << value; }
    return os << "]";
}