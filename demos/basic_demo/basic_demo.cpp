#include <iostream>
#include <mpi.h>
#include <print>
#include <ranges>
#include <vector>

#include <block_wise.hpp>
#include <reshuffle.hpp>


using Matrix = std::vector<std::vector<int>>;

enum class ToGridError { MISMATCH_DIMENSIONS_AND_NUM_VALUES };

[[nodiscard]] auto init_matrix(int num_rows, int num_columns) -> Matrix;
[[nodiscard]] auto to_matrix(const std::vector<int> &values,
                             const reshuffle::Dimensions<2> &dimensions)
        -> std::expected<Matrix, ToGridError>;
auto print_domain_decomposition(const reshuffle::Context<2> &current_context,
                                const reshuffle::Dimensions<2> &local_dimensions,
                                const reshuffle::Dimensions<2> &global_dimensions) -> void;

auto operator<<(std::ostream &os, const Matrix &matrix) -> std::ostream &;

int main() {
    constexpr auto num_rows = 20;
    constexpr auto num_columns = 20;

    MPI_Init(nullptr, nullptr);

    if (const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD).value();
        num_ranks != 4) {
        const std::string error_msg =
                "Please run with 4 ranks. Currently running with " + std::to_string(num_ranks);
        std::cerr << error_msg << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    const auto global_matrix =
            reshuffle::mpi::is_root(MPI_COMM_WORLD) ? init_matrix(num_rows, num_columns) : Matrix{};
    constexpr auto global_dimensions = reshuffle::Dimensions<2>{num_rows, num_columns};

    const auto contexts = std::vector{
            reshuffle::Context{reshuffle::distribution::get_all_values_in_root(global_dimensions),
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{global_dimensions,
                                                                  reshuffle::ProcessorGrid{4, 1}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{global_dimensions,
                                                                  reshuffle::ProcessorGrid{1, 4}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockWise{global_dimensions,
                                                                  reshuffle::ProcessorGrid{2, 2}},
                               MPI_COMM_WORLD},
            reshuffle::Context{reshuffle::distribution::BlockCyclic{global_dimensions,
                                                                    reshuffle::Dimensions<2>{5, 5},
                                                                    reshuffle::ProcessorGrid{2, 2}},
                               MPI_COMM_WORLD},
    };

    if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
        std::cout << "\n\n*******************************************************\n\n";

        std::cout << "Initial matrix" << std::endl;
        std::cout << global_matrix << std::endl;
    }

    auto matrix = global_matrix;
    for (auto i = 1; i < contexts.size(); ++i) {
        const auto [new_values, new_dimensions] =
                reshuffle::shuffle(matrix, contexts[i - 1], contexts[i]);
        matrix = to_matrix(new_values, new_dimensions).value();
        if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
            std::cout << "\n\n****************** Shuffle " << i << " *************************\n\n";
            std::cout << "Values in Rank 0" << std::endl;
            std::cout << matrix << std::endl;
        }

        print_domain_decomposition(contexts[i], new_dimensions, global_dimensions);
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

auto to_matrix(const std::vector<int> &values, const reshuffle::Dimensions<2> &dimensions)
        -> std::expected<Matrix, ToGridError> {

    if (reshuffle::internal::calc_total_num_values(dimensions) != values.size()) {
        return std::unexpected(ToGridError::MISMATCH_DIMENSIONS_AND_NUM_VALUES);
    }

    const auto as_span = std::mdspan(values.data(), dimensions);

    auto grid = Matrix(dimensions[0], std::vector(dimensions[1], 0));
    for (int i = 0; i < dimensions[0]; i++) {
        for (int j = 0; j < dimensions[1]; j++) { grid[i][j] = as_span[i, j]; }
    }

    return grid;
}

auto print_domain_decomposition(const reshuffle::Context<2> &current_context,
                                const reshuffle::Dimensions<2> &local_dimensions,
                                const reshuffle::Dimensions<2> &global_dimensions) -> void {

    constexpr auto rank_to_char = std::array{'#', '@', '*', '$'};
    const auto rank = reshuffle::mpi::get_rank_id(MPI_COMM_WORLD).value();

    const unsigned long num_rows = local_dimensions[0];
    const unsigned long num_columns = local_dimensions[1];
    const auto local_decomposition =
            std::vector{num_rows, std::vector(num_columns, rank_to_char[rank])};

    const auto all_in_root = reshuffle::Context{
            reshuffle::distribution::BlockWise{global_dimensions, reshuffle::ProcessorGrid{1, 1}},
            MPI_COMM_WORLD};

    const auto [global_decomposition_values, dimensions] =
            reshuffle::shuffle(local_decomposition, current_context, all_in_root);

    if (reshuffle::mpi::is_root(MPI_COMM_WORLD)) {
        std::println("Domain Decomposition: Rank 0: {}, Rank 1: {}, Rank 2: {}, Rank 3: {}",
                     rank_to_char[0], rank_to_char[1], rank_to_char[2], rank_to_char[3]);
        const auto matrix_view =
                std::mdspan{global_decomposition_values.data(), dimensions[0], dimensions[1]};
        for (int i = 0; i < dimensions[0]; i++) {
            auto delimiter = std::string{};
            for (int j = 0; j < dimensions[1]; j++) {
                std::cout << std::exchange(delimiter, " ") << std::setfill(matrix_view[i, j])
                          << std::setw(3) << matrix_view[i, j];
            }
            std::cout << std::endl;
        }
    }
}

auto operator<<(std::ostream &os, const Matrix &matrix) -> std::ostream & {
    for (const auto &row: matrix) {
        for (const auto &value: row) { os << std::setfill('0') << std::setw(3) << value << " "; }
        os << std::endl;
    }

    return os;
}
