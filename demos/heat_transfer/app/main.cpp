#include <iostream>

#include <grid_operations.hpp>
#include <vtk_writer.hpp>

int main() {
    constexpr auto num_iterations = 200;
    constexpr auto num_rows = 100;
    constexpr auto num_columns = num_rows;

    const auto output_folder = std::filesystem::current_path() / "output";
    const auto files_prefix = "vtk_output_";

    const auto writer = heat::vtk::VTKWriter{output_folder, files_prefix};

    auto grid = heat::initialize_grid(num_rows, num_columns);
    for (auto i = 0; i < num_iterations; i++) {
        writer.record_timestep(i, grid);
        grid = heat::apply_jacobi(grid);
    }

    return 0;
}