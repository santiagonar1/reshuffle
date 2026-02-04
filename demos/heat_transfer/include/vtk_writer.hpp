#ifndef RESHUFFLE_VTK_WRITER_HPP
#define RESHUFFLE_VTK_WRITER_HPP

#include <filesystem>

#include "matrix.hpp"

namespace heat::vtk {

    auto write_header(std::ostream &output, unsigned int nx, unsigned int ny) -> void;
    auto write_data(std::ostream &output, const Matrix2D &grid, int precision = 5) -> void;
    auto write_file(std::ostream &output, const Matrix2D &grid) -> void;
    auto write_file(const std::filesystem::path &path, const Matrix2D &grid) -> void;
    auto create_folder(const std::filesystem::path &path) -> void;

}// namespace heat::vtk

#endif//RESHUFFLE_VTK_WRITER_HPP
