#ifndef RESHUFFLE_VTK_WRITER_HPP
#define RESHUFFLE_VTK_WRITER_HPP

#include <filesystem>

#include <rank_id.hpp>

#include "matrix.hpp"

namespace heat::vtk {
    class VTKWriter {
    public:
        VTKWriter(const std::filesystem::path &output_folder, std::string files_prefix);

        auto record_timestep(unsigned int current_iteration, const Matrix2D &grid,
                             reshuffle::RankId rank) const -> void;

        static auto write_file(std::ostream &output, const Matrix2D &grid) -> void;
        static auto write_header(std::ostream &output, unsigned int ny, unsigned int nx) -> void;
        static auto write_data(std::ostream &output, const Matrix2D &grid, int precision = 5)
                -> void;

    private:
        const std::filesystem::path _output_folder;
        const std::string _files_prefix;
    };

}// namespace heat::vtk

#endif//RESHUFFLE_VTK_WRITER_HPP
