#ifndef RESHUFFLE_VTK_WRITER_HPP
#define RESHUFFLE_VTK_WRITER_HPP

#include <filesystem>
#include <optional>

#include <rank_id.hpp>

#include "grid.hpp"

namespace heat::vtk {
    class VTKWriter {
    public:
        VTKWriter(const std::filesystem::path &output_folder, std::string files_prefix);

        auto record_timestep(unsigned int current_iteration, const Grid &grid,
                             reshuffle::RankId rank,
                             const std::optional<RankGrid> &rank_grid = std::nullopt) const -> void;

        static auto write_file(std::ostream &output, const Grid &grid,
                               const std::optional<RankGrid> &rank_grid = std::nullopt) -> void;
        static auto write_header(std::ostream &output, unsigned int ny, unsigned int nx) -> void;
        static auto write_data(std::ostream &output, const Grid &grid, int precision = 5) -> void;
        static auto write_rank_data(std::ostream &output, const RankGrid &rank_grid) -> void;

    private:
        const std::filesystem::path _output_folder;
        const std::string _files_prefix;
    };

}// namespace heat::vtk

#endif//RESHUFFLE_VTK_WRITER_HPP
