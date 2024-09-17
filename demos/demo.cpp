#include <iostream>
#include <mpi.h>
#include <ranges>
#include <vector>

#include <reshuffle.hpp>


using Matrix = std::vector<std::vector<int>>;
using DataDistribution2D = std::array<reshuffle::BlockCyclic, 2>;

bool is_root();

void print(const Matrix &matrix);

Matrix init_matrix(int num_rows, int num_columns);

int main() {
    constexpr int num_rows = 20;
    constexpr int num_columns = 20;
    constexpr int num_values = num_rows * num_columns;

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

    auto matrix = is_root() ? init_matrix(num_rows, num_columns) : Matrix{};
    const auto initial_global_coloring = std::vector<reshuffle::rank_id>(num_values, 0);
    const std::vector<DataDistribution2D> distributions = {
            {reshuffle::make_block_wise(num_columns, 1), reshuffle::make_block_wise(num_rows, 1)},
            {reshuffle::make_block_wise(num_columns, 4), reshuffle::make_block_wise(num_rows, 1)},
            {reshuffle::make_block_wise(num_columns, 2), reshuffle::make_block_wise(num_rows, 2)},
            {reshuffle::make_block_wise(num_columns, 1), reshuffle::make_block_wise(num_rows, 4)}};


    for (int i = 1; i < distributions.size(); ++i) {
        matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, distributions[i - 1], distributions[i]);
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

Matrix init_matrix(const int num_rows, const int num_columns) {
    auto matrix = Matrix(num_rows, std::vector<int>(num_columns));

    int counter{};
    for (auto &row: matrix) {
        for (auto &value: row) {
            value = counter;
            counter++;
        }
    }

    return matrix;
}
