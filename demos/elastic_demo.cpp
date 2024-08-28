#include <iostream>
#include <mpi.h>
#include <vector>

#include <reshuffle.hpp>


bool is_root(const MPI_Comm &comm = MPI_COMM_WORLD);
bool is_rank_active(int num_active_ranks);
reshuffle::rank_id get_rank(const MPI_Comm &comm = MPI_COMM_WORLD);
int get_num_ranks(const MPI_Comm &comm = MPI_COMM_WORLD);

MPI_Comm simulate_adaptation(int num_active_ranks);

int main() {
    constexpr int num_elements = 10;

    MPI_Init(nullptr, nullptr);

    const auto available_ranks = get_num_ranks();
    auto buffer = std::vector<int>(num_elements, get_rank());

    for (int active_ranks = 1; active_ranks <= available_ranks; active_ranks++) {
        auto comm = simulate_adaptation(active_ranks);
        buffer = reshuffle::shuffle(buffer, MPI_COMM_WORLD, comm);

        if (is_rank_active(active_ranks)) {
            if (is_root(comm)) {
                std::ranges::for_each(buffer, [](auto v) { std::cout << v << ", "; });
                std::cout << std::endl;
            }
            MPI_Comm_free(&comm);
        }
    }


    MPI_Finalize();
    return 0;
}

MPI_Comm simulate_adaptation(const int num_active_ranks) {
    MPI_Barrier(MPI_COMM_WORLD);

    const auto color = is_rank_active(num_active_ranks) ? 1 : MPI_UNDEFINED;
    const auto rank = get_rank();

    MPI_Comm comm_after_adaptation{};
    MPI_Comm_split(MPI_COMM_WORLD, color, rank, &comm_after_adaptation);

    return comm_after_adaptation;
}

bool is_root(const MPI_Comm &comm) {
    const auto rank = get_rank(comm);
    return rank == 0;
}

bool is_rank_active(const int num_active_ranks) {
    const auto rank = get_rank();
    return rank < num_active_ranks;
}

reshuffle::rank_id get_rank(const MPI_Comm &comm) {
    reshuffle::rank_id id{};
    MPI_Comm_rank(comm, &id);

    return id;
}

int get_num_ranks(const MPI_Comm &comm) {
    int num_ranks{};
    MPI_Comm_size(comm, &num_ranks);

    return num_ranks;
}