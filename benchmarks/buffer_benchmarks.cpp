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
#include <numeric>
#include <ranges>

#include <reshuffle.hpp>

bool is_root();
std::vector<int> get_num_elements_per_rank(int total_num_elements);
reshuffle::rank_id get_rank();

constexpr int NUM_ELEMENTS = 2000;

void reorder_data(benchmark::State &state) {
    const auto rank = get_rank();

    const auto num_elements_per_rank = get_num_elements_per_rank(NUM_ELEMENTS);
    const auto original_data = std::vector<int>(num_elements_per_rank[rank]);
    const auto initial_global_coloring = std::vector<reshuffle::rank_id>(NUM_ELEMENTS, 0);

    double max_elapsed_second{};
    while (state.KeepRunning()) {
        // Do the work and time it on each proc
        auto start = std::chrono::high_resolution_clock::now();
        auto data = reshuffle::shuffle(original_data, MPI_COMM_WORLD);
        auto end = std::chrono::high_resolution_clock::now();
        // Now get the max time across all procs:
        // for better or for worse, the slowest processor is the one that is
        // holding back the others in the benchmark.
        auto const duration =
                std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        auto elapsed_seconds = duration.count();
        MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX,
                      MPI_COMM_WORLD);
        state.SetIterationTime(max_elapsed_second);
    }
}

void shuffle_from_one_to_N(benchmark::State &state) {
    constexpr auto num_values = 2000;
    const auto original_data = reshuffle::internal::is_root(MPI_COMM_WORLD)
                                       ? std::vector<int>(num_values)
                                       : std::vector<int>{};

    while (state.KeepRunning()) {
        // Do the work and time it on each proc
        auto start = std::chrono::high_resolution_clock::now();
        auto data = reshuffle::shuffle(original_data, MPI_COMM_WORLD);
        auto end = std::chrono::high_resolution_clock::now();

        // Now get the max time across all procs:
        // for better or for worse, the slowest processor is the one that is
        // holding back the others in the benchmark.
        auto const duration =
                std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
        auto elapsed_seconds = duration.count();

        double max_elapsed_second{};
        MPI_Allreduce(&elapsed_seconds, &max_elapsed_second, 1, MPI_DOUBLE, MPI_MAX,
                      MPI_COMM_WORLD);
        state.SetIterationTime(max_elapsed_second);
    }
}

BENCHMARK(reorder_data)->UseManualTime();
BENCHMARK(shuffle_from_one_to_N)->UseManualTime();

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

    if (is_root())
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

bool is_root() { return get_rank() == 0; }

reshuffle::rank_id get_rank() {
    reshuffle::rank_id rank{};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    return rank;
}

std::vector<int> get_num_elements_per_rank(int total_num_elements) {
    int num_ranks{};
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    const auto seq = std::views::iota(0, num_ranks);
    const auto rank_id_sum = std::accumulate(seq.begin(), seq.end(), 0);

    std::vector<double> weights_per_rank(num_ranks);
    std::ranges::transform(seq, weights_per_rank.begin(),
                           [rank_id_sum](auto r) { return static_cast<double>(r) / rank_id_sum; });

    std::vector<int> num_elements_per_rank(num_ranks);
    std::ranges::transform(
            weights_per_rank, num_elements_per_rank.begin(),
            [total_num_elements](auto w) { return static_cast<int>(total_num_elements * w); });

    const auto num_missing_elements =
            total_num_elements -
            std::accumulate(num_elements_per_rank.begin(), num_elements_per_rank.end(), 0);

    num_elements_per_rank[0] += num_missing_elements;

    return num_elements_per_rank;
}