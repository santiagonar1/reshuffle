// Taken from: https://gist.github.com/mdavezac/eb16de7e8fc08e522ff0d420516094f5

// Distributed under the MIT license
// The following demonstrates how to use google/benchmark with MPI enabled
// codes. There are three core aspects:
//
// 1. The time it takes to run a single iteration of a particular benchmark is
//    the maximum time across all processes. In a two process job, if process 0
//    takes 1 second to do its work, and process 1 takes 1.5 seconds, then 1.5
//    seconds is the time recorded for benchmarking purposes.
// 2. Only the root process is allowed to report. Other processes report via a
//    "NullReporter".
// 3. The main is modified to initialize and finalize MPI. It is also modified
// for the purpose of point 2 above.
//
// Note: The efficiency of MPI algorithm often depends on interweaving compute
// and communication. Depending on how it is applied, benchmarking, and
// especially micro-benchmarking, may not capture this aspect.
//


#include <benchmark/benchmark.h>
#include <chrono>
#include <mpi.h>

#include <reshuffle.hpp>

using SendType = int;

double time_shuffle(const std::vector<SendType> &values,
                    const reshuffle::BlockCyclic &current_distribution,
                    const reshuffle::BlockCyclic &new_distribution) {
    // Do the work and time it on each proc
    const auto start = std::chrono::high_resolution_clock::now();
    const auto _ =
            reshuffle::shuffle(values, MPI_COMM_WORLD, current_distribution, new_distribution);
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
    const auto num_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::internal::get_num_ranks(MPI_COMM_WORLD);

    if (num_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto current_distribution = reshuffle::make_block_wise(num_values, num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(num_values, 1);

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, current_distribution, new_distribution));
    }
}

void shuffle_from_one_to_N_with_distribution(benchmark::State &state) {
    const auto num_values = static_cast<int>(state.range(0));
    const auto original_values = reshuffle::internal::is_root(MPI_COMM_WORLD)
                                         ? std::vector<SendType>(num_values)
                                         : std::vector<SendType>{};

    const auto num_ranks = reshuffle::internal::get_num_ranks(MPI_COMM_WORLD);

    const auto current_distribution = reshuffle::make_block_wise(num_values, 1);
    const auto new_distribution = reshuffle::make_block_wise(num_values, num_ranks);

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, current_distribution, new_distribution));
    }
}

void shuffle_from_N_to_N(benchmark::State &state) {
    const auto num_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::internal::get_num_ranks(MPI_COMM_WORLD);

    if (num_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    const auto values_per_rank = num_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto current_distribution = reshuffle::make_block_wise(num_values, num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(num_values, num_ranks);

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, current_distribution, new_distribution));
    }
}

void shuffle_reduction(benchmark::State &state) {
    const auto num_values = static_cast<int>(state.range(0));

    const auto num_ranks = reshuffle::internal::get_num_ranks(MPI_COMM_WORLD);

    if (num_values % num_ranks != 0) {
        throw std::runtime_error("Number of values not divisible by number of ranks");
    }

    if (num_ranks < 2) {
        throw std::runtime_error("You need to use at least two ranks for this benchmark");
    }

    const auto values_per_rank = num_values / num_ranks;
    const auto original_values = std::vector<SendType>(values_per_rank);

    const auto current_distribution = reshuffle::make_block_wise(num_values, num_ranks);
    const auto new_distribution = reshuffle::make_block_wise(num_values, num_ranks / 2);

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, current_distribution, new_distribution));
    }
}

BENCHMARK(shuffle_from_N_to_one)->UseManualTime()->DenseRange(1000, 100000, 10000);
BENCHMARK(shuffle_from_one_to_N_with_distribution)
        ->UseManualTime()
        ->DenseRange(1000, 100000, 10000);
BENCHMARK(shuffle_reduction)->UseManualTime()->DenseRange(1000, 10000, 1000);
BENCHMARK(shuffle_from_N_to_N)->UseManualTime()->DenseRange(1000, 100000, 10000);

// This reporter does nothing.
// We can use it to disable output from all but the root process
class NullReporter final : public benchmark::BenchmarkReporter {
public:
    NullReporter() = default;
    bool ReportContext(const Context &) override { return true; }
    void ReportRuns(const std::vector<Run> &) override {}
    void Finalize() override {}
};

// The main is rewritten to allow for MPI initializing and for selecting a
// reporter according to the process rank
int main(int argc, char **argv) {

    MPI_Init(&argc, &argv);

    benchmark::Initialize(&argc, argv);

    if (reshuffle::internal::is_root(MPI_COMM_WORLD))
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