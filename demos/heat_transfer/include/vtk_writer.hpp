#ifndef RESHUFFLE_VTK_WRITER_HPP
#define RESHUFFLE_VTK_WRITER_HPP

#include <filesystem>

#include "matrix.hpp"

namespace heat::vtk {

    auto write_header(std::ostream &output, unsigned int nx, unsigned int ny) -> void;
    auto write_data(std::ostream &output, const Matrix2D &grid, int precision = 5) -> void;

}// namespace heat::vtk

#endif//RESHUFFLE_VTK_WRITER_HPP
