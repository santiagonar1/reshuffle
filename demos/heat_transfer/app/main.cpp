#include <expected>
#include <iostream>
#include <mpi.h>

#include <reshuffle.hpp>

#include <grid_operations.hpp>
#include <processor_info.hpp>
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
    constexpr auto global_dimensions = reshuffle::Dimensions{num_rows, num_columns};

    MPI_Init(&argc, &argv);

    const auto processor_grid = get_processor_grid();
    auto cartesian_comm = get_cartesian_comm(MPI_COMM_WORLD, processor_grid).value();
    const auto processor_info = heat::ProcessorInfo{cartesian_comm};

    const auto output_folder = std::filesystem::current_path() / "output";
    const auto files_prefix = "vtk_output_";

    const auto writer = heat::vtk::VTKWriter{output_folder, files_prefix};

    const auto global_grid = reshuffle::mpi::is_root(cartesian_comm)
                                     ? heat::initialize_grid(num_rows, num_columns)
                                     : heat::Matrix2D{};

    const auto initial_context = reshuffle::Context<2>{
            reshuffle::BlockWise<2>{global_dimensions, reshuffle::ProcessorGrid{1, 1}},
            cartesian_comm};
    const auto final_context = reshuffle::Context<2>{
            reshuffle::BlockWise<2>{global_dimensions, processor_grid}, cartesian_comm};

    auto local_grid = heat::add_ghost_layers(
            heat::to_grid(reshuffle::shuffle(global_grid, initial_context, final_context)).value(),
            processor_info);

    for (auto i = 0; i < num_iterations; i++) {
        local_grid = heat::exchange_ghost_layers(local_grid, processor_info, cartesian_comm);
        if (reshuffle::mpi::is_root(cartesian_comm)) {
            writer.record_timestep(i, heat::remove_ghost_layers(local_grid, processor_info));
        }
        local_grid = heat::apply_jacobi(local_grid);
    }

    MPI_Comm_free(&cartesian_comm);
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