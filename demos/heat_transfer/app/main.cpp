#include <expected>
#include <mpi.h>

#include <grid_operations.hpp>
#include <mpi_utils.hpp>
#include <processor_grid.hpp>
#include <vtk_writer.hpp>

enum class GetCartesianCommError {
    INVALID_PROCESSOR_GRID,
};

auto get_processor_grid() -> reshuffle::ProcessorGrid<2>;
auto get_cartesian_comm(MPI_Comm base_comm, reshuffle::ProcessorGrid<2> processor_grid)
        -> std::expected<MPI_Comm, GetCartesianCommError>;

int main(int argc, char *argv[]) {
    constexpr auto num_iterations = 200;
    constexpr auto num_rows = 100;
    constexpr auto num_columns = num_rows;

    MPI_Init(&argc, &argv);

    const auto processor_grid = get_processor_grid();
    const auto cartesian_comm = get_cartesian_comm(MPI_COMM_WORLD, processor_grid).value();

    const auto output_folder = std::filesystem::current_path() / "output";
    const auto files_prefix = "vtk_output_";

    const auto writer = heat::vtk::VTKWriter{output_folder, files_prefix};

    auto grid = heat::initialize_grid(num_rows, num_columns);
    for (auto i = 0; i < num_iterations; i++) {
        writer.record_timestep(i, grid);
        grid = heat::apply_jacobi(grid);
    }

    MPI_Finalize();
    return 0;
}

auto get_processor_grid() -> reshuffle::ProcessorGrid<2> {
    constexpr auto num_dimensions = 2;
    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

    auto dimensions = reshuffle::Dimensions<num_dimensions>{};
    MPI_Dims_create(num_ranks, num_dimensions, dimensions.data());

    return reshuffle::ProcessorGrid{dimensions};
}

auto get_cartesian_comm(const MPI_Comm base_comm, const reshuffle::ProcessorGrid<2> processor_grid)
        -> std::expected<MPI_Comm, GetCartesianCommError> {
    constexpr auto num_dimensions = 2;
    const auto num_available_ranks = reshuffle::mpi::get_num_ranks(base_comm);

    if (processor_grid.get_num_processors() != num_available_ranks) {
        return std::unexpected(GetCartesianCommError::INVALID_PROCESSOR_GRID);
    }

    constexpr auto periods = std::array<int, num_dimensions>{};
    constexpr auto reorder = true;
    MPI_Comm cartesian_comm;
    MPI_Cart_create(base_comm, num_dimensions, processor_grid.get_dimensions().data(),
                    periods.data(), reorder, &cartesian_comm);

    return cartesian_comm;
}