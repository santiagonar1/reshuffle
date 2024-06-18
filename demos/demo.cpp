#include <iostream>
#include <mpi.h>
#include <ranges>
#include <vector>

#include <reshuffle.hpp>


using Matrix = std::vector<std::vector<int>>;
using DataDistribution2D = std::array<reshuffle::BlockWise, 2>;

bool is_root();

void print(const Matrix &matrix);

int main() {
    constexpr int num_rows = 20;
    constexpr int num_columns = 20;
    constexpr reshuffle::Dimensions2D global_dimension{num_rows, num_columns};
    constexpr int num_elements = num_rows * num_columns;

    MPI_Init(nullptr, nullptr);

    int rank{};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    int num_ranks{};
    MPI_Comm_size(MPI_COMM_WORLD, &num_ranks);

    if (num_ranks != 4) {
        const std::string error_msg =
                "Please run with 4 ranks. Currently running with " + std::to_string(num_ranks);
        throw std::invalid_argument(error_msg);
    }

    auto matrix = is_root() ? Matrix(num_rows, std::vector<int>(num_columns, 3)) : Matrix{};
    const auto initial_global_coloring = std::vector<reshuffle::rank_id>(num_elements, 0);
    const std::vector<DataDistribution2D> distributions = {
            {reshuffle::BlockWise(4), reshuffle::BlockWise(1)},
            {reshuffle::BlockWise(2), reshuffle::BlockWise(2)},
            {reshuffle::BlockWise(1), reshuffle::BlockWise(4)}};


    auto global_coloring = initial_global_coloring;
    auto local_coloring = std::vector<reshuffle::rank_id>{};
    for (const auto &distribution: distributions) {
        std::tie(global_coloring, local_coloring) =
                reshuffle::create_coloring(global_coloring, global_dimension, distribution, rank)
                        .as_tuple();
        const auto block_dimension =
                reshuffle::get_block_dimension(distribution, global_dimension, rank);
        matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, local_coloring, block_dimension);
        if (is_root()) {
            print(matrix);
            std::cout << "\n";
        }
    }

    MPI_Finalize();
    return 0;
}

bool is_root() {
    int rank{};
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    return rank == 0;
}

void print(const Matrix &matrix) {
    for (const auto &row: matrix) {
        for (const auto &value: row) { std::cout << value << " "; }
        std::cout << std::endl;
    }
}
