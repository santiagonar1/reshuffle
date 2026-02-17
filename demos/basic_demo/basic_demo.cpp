#include <iostream>
#include <mpi.h>
#include <ranges>
#include <vector>

#include <block_wise.hpp>
#include <reshuffle.hpp>


using Matrix = std::vector<std::vector<int>>;

auto init_matrix(int num_rows, int num_columns) -> Matrix;
auto init_vector(int num_values) -> std::vector<int>;

auto operator<<(std::ostream &os, const Matrix &matrix) -> std::ostream &;

int main() {
    constexpr auto num_rows = 20;
    constexpr auto num_columns = 20;
    constexpr auto num_values = num_rows * num_columns;

    MPI_Init(nullptr, nullptr);

    if (const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD); num_ranks != 4) {
        const std::string error_msg =
                "Please run with 4 ranks. Currently running with " + std::to_string(num_ranks);
        std::cerr << error_msg << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    auto matrix =
            reshuffle::mpi::is_root(MPI_COMM_WORLD) ? init_vector(num_values) : std::vector<int>{};
    auto dimensions = reshuffle::Dimensions<2>{num_rows, num_columns};

    const auto contexts = std::vector{
            reshuffle::Context{reshuffle::distribution::BlockWise{
                                       reshuffle::Dimensions<2>{num_rows, num_columns},
                                       reshuffle::ProcessorGrid{1, 1}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{
                                       reshuffle::Dimensions<2>{num_rows, num_columns},
                                       reshuffle::ProcessorGrid{4, 1}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{
                                       reshuffle::Dimensions<2>{num_rows, num_columns},
                                       reshuffle::ProcessorGrid{1, 4}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{
                                       reshuffle::Dimensions<2>{num_rows, num_columns},
                                       reshuffle::ProcessorGrid{2, 2}},
                               MPI_COMM_WORLD},
    };

    if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
        auto counter = 0;
        for (auto rowIndex = 0; rowIndex < dimensions[0]; ++rowIndex) {
            for (auto columnIndex = 0; columnIndex < dimensions[1]; ++columnIndex) {
                std::cout << matrix[counter] << " ";
                counter++;
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    for (auto i = 1; i < contexts.size(); ++i) {
        const auto [new_matrix, new_dimensions] =
                reshuffle::shuffle(std::mdspan{std::as_const(matrix).data(), dimensions},
                                   contexts[i - 1], contexts[i]);
        matrix = new_matrix;
        dimensions = new_dimensions;
        if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
            auto counter = 0;
            for (auto rowIndex = 0; rowIndex < dimensions[0]; ++rowIndex) {
                for (auto columnIndex = 0; columnIndex < dimensions[1]; ++columnIndex) {
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

auto init_matrix(const int num_rows, const int num_columns) -> Matrix {
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

auto init_vector(const int num_values) -> std::vector<int> {
    auto values = std::vector<int>(num_values);
    std::iota(values.begin(), values.end(), 0);
    return values;
}

auto operator<<(std::ostream &os, const Matrix &matrix) -> std::ostream & {
    for (const auto &row: matrix) {
        for (const auto &value: row) { os << value << " "; }
        os << std::endl;
    }

    return os;
}
