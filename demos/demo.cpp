#include <iostream>
#include <mpi.h>
#include <ranges>
#include <vector>

#include <block_wise.hpp>
#include <reshuffle.hpp>


using Matrix = std::vector<std::vector<int>>;

bool is_root();

Matrix init_matrix(int num_rows, int num_columns);
auto init_vector(int num_values) -> std::vector<int>;

std::ostream &operator<<(std::ostream &os, const Matrix &matrix);

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

    auto matrix = is_root() ? init_vector(num_values) : std::vector<int>{};
    auto dimensions = reshuffle::Dimensions<2>{num_rows, num_columns};

    const std::vector contexts = {
            reshuffle::Context{reshuffle::BlockWise{reshuffle::Dimensions<2>{num_rows, num_columns},
                                                    reshuffle::ProcessorGrid{1, 1}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::BlockWise{reshuffle::Dimensions<2>{num_rows, num_columns},
                                                    reshuffle::ProcessorGrid{4, 1}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::BlockWise{reshuffle::Dimensions<2>{num_rows, num_columns},
                                                    reshuffle::ProcessorGrid{1, 4}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::BlockWise{reshuffle::Dimensions<2>{num_rows, num_columns},
                                                    reshuffle::ProcessorGrid{2, 2}},
                               MPI_COMM_WORLD},
    };

    if (is_root()) {
        int counter = 0;
        for (int rowIndex = 0; rowIndex < dimensions[0]; ++rowIndex) {
            for (int columnIndex = 0; columnIndex < dimensions[1]; ++columnIndex) {
                std::cout << matrix[counter] << " ";
                counter++;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    for (int i = 1; i < contexts.size(); ++i) {
        const auto [new_matrix, new_dimensions] =
                reshuffle::shuffle(std::mdspan{std::as_const(matrix).data(), dimensions},
                                   contexts[i - 1], contexts[i]);
        matrix = new_matrix;
        dimensions = new_dimensions;
        if (is_root()) {
            int counter = 0;
            for (int rowIndex = 0; rowIndex < dimensions[0]; ++rowIndex) {
                for (int columnIndex = 0; columnIndex < dimensions[1]; ++columnIndex) {
                    std::cout << matrix[counter] << " ";
                    counter++;
                }
                std::cout << std::endl;
            }
            std::cout << std::endl;
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

auto init_vector(int num_values) -> std::vector<int> {
    auto values = std::vector<int>(num_values);
    std::iota(values.begin(), values.end(), 0);
    return values;
}

std::ostream &operator<<(std::ostream &os, const Matrix &matrix) {
    for (const auto &row: matrix) {
        for (const auto &value: row) { os << value << " "; }
        os << std::endl;
    }

    return os;
}
