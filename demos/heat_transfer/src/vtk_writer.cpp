#include "vtk_writer.hpp"

#include <fstream>

namespace heat::vtk {

    auto write_header(std::ostream &output, const unsigned int nx, const unsigned int ny) -> void {
        output << "# vtk DataFile Version 4.1\n";
        output << "vtk output\n";
        output << "ASCII\n";
        output << "DATASET STRUCTURED_POINTS\n";
        output << "DIMENSIONS " << nx << " " << ny << " 1\n";
        output << "SPACING 1 1 1\n";
        output << "ORIGIN 0 0 0\n";
        output << "POINT_DATA " << nx * ny << "\n";
        output << "SCALARS ScalarField double\n";
        output << "LOOKUP_TABLE default\n";
    }

    auto write_data(std::ostream &output, const Matrix2D &grid, const int precision) -> void {
        for (const auto &row: grid) {
            auto delimiter = std::string{};
            for (const auto &value: row) {
                output << std::fixed << std::setprecision(precision)
                       << std::exchange(delimiter, " ") << value;
            }
            output << std::endl;
        }
    }

    auto write_file(std::ostream &output, const Matrix2D &grid) -> void {
        const auto nx = grid.size();
        const auto ny = grid[0].size();

        write_header(output, nx, ny);
        write_data(output, grid);
    }

    auto write_file(const std::filesystem::path &path, const Matrix2D &grid) -> void {
        auto vtk_file = std::ofstream{path};

        if (not vtk_file.is_open()) {
            throw std::runtime_error{"Failed to open file for writing: " + path.string()};
        }

        write_file(vtk_file, grid);
    }

    auto create_folder(const std::filesystem::path &path) -> void {
        std::filesystem::create_directories(path);
    }

}// namespace heat::vtk
