#include <iostream>
#include <mpi.h>
#include <vector>

#include <reshuffle.hpp>


int main() {
    using SendType = double;

    constexpr auto num_global_values = 990'000'000;

    MPI_Init(nullptr, nullptr);

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);
    if (num_global_values % num_ranks != 0) {
        const auto error_msg =
                std::format("Number of values {} not divisible by number of ranks {}",
                            num_global_values, num_ranks);
        std::cerr << error_msg << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const auto local_values = reshuffle::mpi::is_root(MPI_COMM_WORLD)
                                      ? std::vector<SendType>(num_global_values)
                                      : std::vector<SendType>{};

    const auto initial_processor_grid = reshuffle::ProcessorGrid{1};
    const auto initial_distribution =
            reshuffle::BlockWise{{num_global_values}, initial_processor_grid};
    const auto initial_context = reshuffle::Context{initial_distribution, MPI_COMM_WORLD};

    const auto final_processor_grid = reshuffle::ProcessorGrid{num_ranks};
    const auto final_distribution =
            reshuffle::BlockCyclic{{num_global_values}, {1000}, final_processor_grid};
    const auto final_context = reshuffle::Context{final_distribution, MPI_COMM_WORLD};

    const auto _ = shuffle(std::mdspan{local_values.data(), local_values.size()}, initial_context,
                           final_context);


#ifdef ENABLE_PROFILING_RESHUFFLE
    tracy::GetProfiler().RequestShutdown();
    while (not tracy::GetProfiler().HasShutdownFinished()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    };
#endif

    MPI_Finalize();
    return 0;
}
