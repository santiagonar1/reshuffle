#include "interval.hpp"
#include "mpi_utils.hpp"
#include "processor_grid.hpp"


#include <mpi.h>

#include <grid_operations.hpp>
#include <vtk_writer.hpp>

auto get_processor_grid() -> reshuffle::ProcessorGrid<2>;

int main(int argc, char *argv[]) {
    constexpr auto num_iterations = 200;
    constexpr auto num_rows = 100;
    constexpr auto num_columns = num_rows;

    MPI_Init(&argc, &argv);

    const auto num_ranks = reshuffle::mpi::get_num_ranks(MPI_COMM_WORLD);

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