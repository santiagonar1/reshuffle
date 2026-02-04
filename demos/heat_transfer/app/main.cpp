#include <iostream>

#include <grid_operations.hpp>
#include <vtk_writer.hpp>

int main() {
    constexpr auto num_iterations = 200;
    constexpr auto num_rows = 100;
    constexpr auto num_columns = num_rows;

    const auto output_folder = std::filesystem::current_path() / "output";
    heat::vtk::create_folder(output_folder);

    auto grid = heat::initialize_grid(num_rows, num_columns);
    for (auto i = 0; i < num_iterations; i++) {
        const auto filename = "heat_transfer_iteration_" + std::to_string(i) + ".vtk";
        const auto output = output_folder / filename;
        heat::vtk::write_file(output, grid);
        grid = heat::apply_jacobi(grid);
    }

    return 0;
}