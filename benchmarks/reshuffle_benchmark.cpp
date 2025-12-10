#include <benchmark/benchmark.h>
#include <chrono>
#include <mpi.h>

#include <block_cyclic.hpp>
#include <block_wise.hpp>
#include <reshuffle.hpp>

#include "autopas_particle.hpp"
#include "null_reporter.hpp"

using namespace reshuffle;

using SendType = double;
using SerializationType = MoleculeLJ;

template<typename T>
double time_shuffle(const std::vector<T> &local_values, const Context<1> &initial_context,
                    const Context<1> &final_context) {
    // Do the work and time it on each proc
    const auto start = std::chrono::high_resolution_clock::now();
    const auto _ = shuffle(std::mdspan{local_values.data(), local_values.size()}, initial_context,
                           final_context);
    const auto end = std::chrono::high_resolution_clock::now();

    // Now get the max time across all procs:
    // for better or for worse, the slowest processor is the one that is
    // holding back the others in the benchmark.
    const auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    const auto elapsed_seconds = duration.count();

    double max_elapsed_second{};
    MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    return max_elapsed_second;
}

template<typename T>
double time_scatter(std::vector<T> &local_values, const std::map<RankId, int> &values_per_rank) {
    // Do the work and time it on each proc
    const auto start = std::chrono::high_resolution_clock::now();
    const auto _ = mpi::block_scatter(std::span{local_values}, values_per_rank, 0, MPI_COMM_WORLD);
    const auto end = std::chrono::high_resolution_clock::now();

    // Now get the max time across all procs:
    // for better or for worse, the slowest processor is the one that is
    // holding back the others in the benchmark.
    const auto duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
    const auto elapsed_seconds = duration.count();

    double max_elapsed_second{};
    MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX, MPI_COMM_WORLD);

    return max_elapsed_second;
}

void shuffle_from_N_to_one(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{1};
    const auto final_distribution = BlockWise{{num_global_values}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_serialization_from_N_to_one(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SerializationType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{1};
    const auto final_distribution = BlockWise{{num_global_values}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_from_one_to_N(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto original_values = reshuffle::mpi::is_root(MPI_COMM_WORLD)
                                         ? std::vector<SendType>(num_global_values)
                                         : std::vector<SendType>{};

    const auto initial_processor_grid = ProcessorGrid{1};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto final_distribution = BlockWise{{num_global_values}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_serialization_from_one_to_N(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto original_values = reshuffle::mpi::is_root(MPI_COMM_WORLD)
                                         ? std::vector<SerializationType>(num_global_values)
                                         : std::vector<SerializationType>{};

    const auto initial_processor_grid = ProcessorGrid{1};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto final_distribution = BlockWise{{num_global_values}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_scatter_from_one_to_N(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    auto original_values = mpi::is_root(MPI_COMM_WORLD)
                                   ? std::vector<SerializationType>(num_global_values)
                                   : std::vector<SerializationType>{};

    const auto num_values_per_rank = num_global_values / num_ranks;
    auto values_per_rank = std::map<RankId, int>{};
    for (int rank = 0; rank < num_ranks; ++rank) { values_per_rank[rank] = num_values_per_rank; }

    while (state.KeepRunning()) {
        state.SetIterationTime(time_scatter(original_values, values_per_rank));
    }
}

void shuffle_from_N_to_N_same_distribution(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, initial_context));
    }
}

void shuffle_from_N_to_N(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto final_distribution = BlockCyclic{{num_global_values}, {10}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_serialization_from_N_to_N(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SerializationType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks};
    const auto final_distribution = BlockCyclic{{num_global_values}, {10}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

void shuffle_reduction(benchmark::State &state) {
    const auto num_global_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    if (num_global_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    if (num_ranks < 2) {
        throw std::runtime_error("You need to use at least two ranks for this benchmark");
    }

    const auto values_per_rank = num_global_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto initial_processor_grid = ProcessorGrid{num_ranks};
    const auto initial_distribution = BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = ProcessorGrid{num_ranks / 2};
    const auto final_distribution = BlockWise{{num_global_values}, final_processor_grid};
    const auto final_context = Context{final_distribution, MPI_COMM_WORLD};

    while (state.KeepRunning()) {
        state.SetIterationTime(time_shuffle(original_values, initial_context, final_context));
    }
}

constexpr auto START = 1'000;
constexpr auto LIMIT = 502'000;
constexpr auto STEP = 50'000;

BENCHMARK(shuffle_from_N_to_one)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_serialization_from_N_to_one)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_from_one_to_N)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_serialization_from_one_to_N)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_scatter_from_one_to_N)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_reduction)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_from_N_to_N_same_distribution)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_from_N_to_N)->UseManualTime()->DenseRange(START, LIMIT, STEP);
BENCHMARK(shuffle_serialization_from_N_to_N)->UseManualTime()->DenseRange(START, LIMIT, STEP);

// The main is rewritten to allow for MPI initializing and for selecting a
// reporter according to the process rank
int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    benchmark::Initialize(&argc, argv);

    if (reshuffle::mpi::is_root(MPI_COMM_WORLD))
        // root process will use a reporter from the usual set provided by
        // ::benchmark
        benchmark::RunSpecifiedBenchmarks();
    else {
        // reporting from other processes is disabled by passing a custom reporter
        NullReporter null;
        benchmark::RunSpecifiedBenchmarks(&null);
    }

    MPI_Finalize();
    return 0;
}