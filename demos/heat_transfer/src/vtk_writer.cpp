#include "vtk_writer.hpp"

#include "grid_operations.hpp"

#include <fstream>

namespace heat::vtk {
    VTKWriter::VTKWriter(const std::filesystem::path &output_folder, std::string files_prefix)
        : _output_folder{output_folder}, _files_prefix(std::move(files_prefix)) {
        std::filesystem::create_directories(output_folder);
    }

    auto VTKWriter::record_timestep(const unsigned int current_iteration, const Matrix2D &grid,
                                    const reshuffle::RankId rank,
                                    const std::optional<Matrix2D> &rank_grid) const -> void {
        if (rank != 0) { return; }

        const auto path =
                _output_folder / (_files_prefix + std::to_string(current_iteration) + ".vtk");
        auto vtk_file = std::ofstream{path};

        if (not vtk_file.is_open()) {
            throw std::runtime_error{"Failed to open file for writing: " + path.string()};
        }

        write_file(vtk_file, grid, rank_grid);
    }

    auto VTKWriter::write_header(std::ostream &output, unsigned int ny, unsigned int nx) -> void {
        output << "# vtk DataFile Version 4.1\n";
        output << "vtk output\n";
        output << "ASCII\n";
        output << "DATASET STRUCTURED_POINTS\n";
        output << "DIMENSIONS " << ny << " " << nx << " 1\n";
        output << "SPACING 1 1 1\n";
        output << "ORIGIN 0 0 0\n";
        output << "POINT_DATA " << nx * ny << "\n";
        output << "SCALARS ScalarField double\n";
        output << "LOOKUP_TABLE default\n";
    }

    auto VTKWriter::write_data(std::ostream &output, const Matrix2D &grid, const int precision)
            -> void {
        for (const auto &row: grid) {
            auto delimiter = std::string{};
            for (const auto &value: row) {
                output << std::fixed << std::setprecision(precision)
                       << std::exchange(delimiter, " ") << value;
            }
            output << std::endl;
        }
    }

    auto VTKWriter::write_rank_data(std::ostream &output, const Matrix2D &rank_grid) -> void {
        output << "SCALARS RankId int\n";
        output << "LOOKUP_TABLE default\n";
        for (const auto &row: rank_grid) {
            auto delimiter = std::string{};
            for (const auto &value: row) {
                output << std::exchange(delimiter, " ") << static_cast<int>(value);
            }
            output << std::endl;
        }
    }

    auto VTKWriter::write_file(std::ostream &output, const Matrix2D &grid,
                               const std::optional<Matrix2D> &rank_grid) -> void {
        const auto [nx, ny] = get_dimensions(grid);

        write_header(output, ny, nx);
        write_data(output, grid);

        if (rank_grid.has_value()) { write_rank_data(output, rank_grid.value()); }
    }

}// namespace heat::vtk
