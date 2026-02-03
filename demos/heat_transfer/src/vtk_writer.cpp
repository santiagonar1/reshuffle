#include "vtk_writer.hpp"

#include <fstream>

namespace heat::vtk {

    auto write_header(std::ostream &output, const int nx, const int ny) -> void {
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

}// namespace heat::vtk
