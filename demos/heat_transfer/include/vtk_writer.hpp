#ifndef RESHUFFLE_VTK_WRITER_HPP
#define RESHUFFLE_VTK_WRITER_HPP

#include <filesystem>

#include "matrix.hpp"

namespace heat::vtk {

    auto write_header(std::ostream &output, int nx, int ny) -> void;

}// namespace heat::vtk

#endif//RESHUFFLE_VTK_WRITER_HPP
