
#include <benchmark/benchmark.h>
#include <iostream>
#include <mpi.h>
#include <vector>

// ScaLAPACK/BLACS function declarations
extern "C" {
void blacs_get_(int *, int *, int *);
void blacs_gridinit_(int *, char *, int *, int *);
void blacs_gridinfo_(int *, int *, int *, int *, int *);
void blacs_gridexit_(int *);
void descinit_(int *, int *, int *, int *, int *, int *, int *, int *, int *, int *);
void pdgeadd_(char *, int *, int *, double *, double *, int *, int *, int *, double *, double *,
              int *, int *, int *);
}

auto find_multiple(int number, int starting_number) -> int;
auto generate_values_only_in_root(int num_values) -> std::vector<double>;

auto is_root(const MPI_Comm &comm = MPI_COMM_WORLD) -> bool;
auto get_rank_id(const MPI_Comm &comm = MPI_COMM_WORLD) -> int;
auto get_num_ranks(const MPI_Comm &comm = MPI_COMM_WORLD) -> int;

auto time_shuffle(std::vector<double> original_values, std::vector<double> &local_vector,
                  std::vector<int> &descriptor_src, std::vector<int> &descriptor_dist,
                  int num_values) -> double {

    double alpha = 1.0;
    double beta = 0.0;
    char trans = 'N';
    int ione = 1;
    const int rank = get_rank_id();

    const auto start = std::chrono::high_resolution_clock::now();
    // Redistribute using pdgeadd
    pdgeadd_(&trans,                                      // No transpose
             &num_values,                                 // Number of rows
             &ione,                                       // Number of columns (1 for vector)
             &alpha,                                      // α = 1.0
             rank == 0 ? original_values.data() : nullptr,// Source data
             &ione,                                       // First row of A
             &ione,                                       // First column of A
             descriptor_src.data(),                       // Source descriptor
             &beta,                                       // β = 0.0
             local_vector.data(),                         // Target data
             &ione,                                       // First row of C
             &ione,                                       // First column of C
             descriptor_dist.data());                     // Distributed descriptor
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

void shuffle_from_one_to_N_with_distribution(benchmark::State &state) {
    const auto rank = get_rank_id();
    const auto num_ranks = get_num_ranks();

    // Vector size
    auto max_num_values = static_cast<int>(state.range(0));
    auto num_values = find_multiple(num_ranks, max_num_values);

    // Initialize source vector on rank 0
    std::vector<double> original_values = generate_values_only_in_root(num_values);

    int context;
    int what = -1, ictxt = 0;
    blacs_get_(&what, &ictxt, &context);

    // BLACS grid initialization (NUM_RANKSx1 process grid)
    char order = 'R';
    int num_procs_row = num_ranks;
    int num_procs_col = 1;// Changed to 4x1 grid
    blacs_gridinit_(&context, &order, &num_procs_row, &num_procs_col);

    // Calculate local matrix dimensions
    int num_local_rows = num_values / num_procs_row;

    // Allocate local arrays
    std::vector<double> local_vec(num_local_rows, 0.0);// Changed size since npcol = 1

    // Create descriptors for source and distributed vectors
    std::vector<int> desc_src(9, 0), desc_dist(9, 0);
    int lld_src = std::max(1, num_values);     // Leading dimension of source vector
    int lld_dist = std::max(1, num_local_rows);// Leading dimension of distributed vector
    int info;

    // Initialize descriptors
    int izero = 0, ione = 1;

    // Descriptor source (all values in rank 0)
    descinit_(desc_src.data(), &num_values, &ione, &num_values, &ione, &izero, &izero, &context,
              &lld_src, &info);

    // Distributed descriptor (all ranks)
    descinit_(desc_dist.data(), &num_values, &ione, &num_local_rows, &ione, &izero, &izero,
              &context, &lld_dist, &info);

    while (state.KeepRunning()) {
        state.SetIterationTime(
                time_shuffle(original_values, local_vec, desc_src, desc_dist, num_values));
    }

    // Cleanup
    blacs_gridexit_(&context);
}

BENCHMARK(shuffle_from_one_to_N_with_distribution)
        ->UseManualTime()
        ->DenseRange(1000, 100000, 10000);

// This reporter does nothing.
// We can use it to disable output from all but the root process
class NullReporter final : public benchmark::BenchmarkReporter {
public:
    NullReporter() = default;
    bool ReportContext(const Context &) override { return true; }
    void ReportRuns(const std::vector<Run> &) override {}
    void Finalize() override {}
};

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    benchmark::Initialize(&argc, argv);

    if (is_root(MPI_COMM_WORLD))
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

auto find_multiple(const int number, const int starting_number) -> int {
    const auto reminder = starting_number % number;
    return starting_number - reminder;
}

auto generate_values_only_in_root(int num_values) -> std::vector<double> {
    std::vector<double> values{};
    if (is_root()) {
        values.resize(num_values);
        for (int i = 0; i < num_values; i++) { values[i] = i + 1; }
    }

    return values;
}

auto is_root(const MPI_Comm &comm) -> bool { return get_rank_id(comm) == 0; }

auto get_rank_id(const MPI_Comm &comm) -> int {
    int id{};
    MPI_Comm_rank(comm, &id);

    return id;
}

auto get_num_ranks(const MPI_Comm &comm) -> int {
    int num_ranks{};
    MPI_Comm_size(comm, &num_ranks);

    return num_ranks;
}
