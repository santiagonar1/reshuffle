#include <benchmark/benchmark.h>
#include <chrono>
#include <mpi.h>

#include <block_cyclic.hpp>
#include <block_wise.hpp>
#include <rank_order.hpp>
#include <reshuffle.hpp>

#include "null_reporter.hpp"

#include <unistd.h>

using namespace reshuffle;
using namespace reshuffle::distribution;

using SendType = double;

template<typename T>
double time_shuffle(const std::vector<T> &local_values, const Context<1> &initial_context,
                    const Context<1> &final_context) {

    const auto start = std::chrono::high_resolution_clock::now();
    const auto _ = shuffle(std::mdspan{local_values.data(), local_values.size()}, initial_context,
                           final_context);
    const auto end = std::chrono::high_resolution_clock::now();

    const auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    const auto elapsed_seconds = duration.count();

    double max_elapsed_second{};
    MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    return max_elapsed_second;
}

void shuffle_from_N_to_N_clustered_communication(benchmark::State &state) {
    // Initial distribution: block-wise across all ranks
    // Final distribution: large blocks that create clustered communication
    // This will cause each rank to send most of its data to only a few other ranks

    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};


    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto large_block_size = num_global_values / (num_ranks * 2);// Much larger blocks
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {large_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_from_N_to_N_clustered_communication_with_relabel(benchmark::State &state) {

    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto large_block_size = num_global_values / (num_ranks * 2);// Much larger blocks
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {large_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto relabeled_comm_optional = get_optimal_communicator(initial_context, final_context);
    const auto relabeled_final_context =
            Context{final_distribution, relabeled_comm_optional.value().first};

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, initial_context, relabeled_final_context));
    }
}

void shuffle_from_N_to_N_clustered_communication_with_relabel_greedy(benchmark::State &state) {

    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto large_block_size = num_global_values / (num_ranks * 2);// Much larger blocks
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {large_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto relabeled_comm_optional =
            get_optimal_communicator_greedy(initial_context, final_context);
    const auto relabeled_final_context =
            Context{final_distribution, relabeled_comm_optional.value().first};

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, initial_context, relabeled_final_context));
    }
}

void shuffle_from_N_to_N_extreme_skew(benchmark::State &state) {
    // Initial distribution: block-wise across all ranks
    // Final distribution: very skewed - first half of data goes to the first quarter of ranks
    // Create a distribution where ranks 0 to num_ranks/4 get most data
    // and ranks num_ranks/4+1 to num_ranks-1 get very little

    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto skewed_block_size = std::max(1, num_global_values / (num_ranks / 4 + 1));
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {skewed_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_from_N_to_N_extreme_skew_with_relabel(benchmark::State &state) {

    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto skewed_block_size = std::max(1, num_global_values / (num_ranks / 4 + 1));
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {skewed_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto relabeled_comm_optional = get_optimal_communicator(initial_context, final_context);
    const auto relabeled_final_context =
            Context{final_distribution, relabeled_comm_optional.value().first};

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, initial_context, relabeled_final_context));
    }
}

void shuffle_from_N_to_N_extreme_skew_with_relabel_greedy(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = mpi::get_num_ranks(MPI_COMM_WORLD).value();

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto skewed_block_size = std::max(1, num_global_values / (num_ranks / 4 + 1));
    const auto final_distribution =
            BlockCyclic{{num_global_values}, {skewed_block_size}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    const auto relabeled_comm_optional =
            get_optimal_communicator_greedy(initial_context, final_context);
    const auto relabeled_final_context =
            Context{final_distribution, relabeled_comm_optional.value().first};

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, initial_context, relabeled_final_context));
    }
}

constexpr auto START = 1'000;
constexpr auto LIMIT = 502'000;
constexpr auto STEP = 50'000;


BENCHMARK(shuffle_from_N_to_N_extreme_skew_with_relabel_greedy)
        ->UseManualTime()
        ->DenseRange(START, LIMIT, STEP)
        ->Repetitions(20)
        ->Unit(benchmark::kMicrosecond);

BENCHMARK(shuffle_from_N_to_N_extreme_skew)
        ->UseManualTime()
        ->DenseRange(START, LIMIT, STEP)
        ->Repetitions(20)
        ->Unit(benchmark::kMicrosecond);


BENCHMARK(shuffle_from_N_to_N_extreme_skew_with_relabel)
        ->UseManualTime()
        ->DenseRange(START, LIMIT, STEP)
        ->Repetitions(20)
        ->Unit(benchmark::kMicrosecond);

BENCHMARK(shuffle_from_N_to_N_clustered_communication)
        ->UseManualTime()
        ->DenseRange(START, LIMIT, STEP)
        ->Repetitions(20)
        ->Unit(benchmark::kMicrosecond);

BENCHMARK(shuffle_from_N_to_N_clustered_communication_with_relabel)
        ->UseManualTime()
        ->DenseRange(START, LIMIT, STEP)
        ->Repetitions(20)
        ->Unit(benchmark::kMicrosecond);


int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);
    benchmark::Initialize(&argc, argv);


    if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) benchmark::RunSpecifiedBenchmarks();
    else {
        NullReporter null;
        benchmark::RunSpecifiedBenchmarks(&null);
    }

    MPI_Finalize();

    return 0;
}