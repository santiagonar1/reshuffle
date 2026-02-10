#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vtk_writer.hpp>

#include <fstream>

using namespace heat::vtk;
using namespace heat;

using testing::Eq;

TEST(VTKWriter, CreatesAnOutputFolder) {
    constexpr auto output_folder = "test_CreatesAnOutputFolder";
    const auto _ = VTKWriter{output_folder, "dummy-prefix"};

    EXPECT_TRUE(std::filesystem::exists(output_folder));
    std::filesystem::remove_all(output_folder);
}

TEST(WriteHeader, WritesHeaderVTKFile) {
    auto ss = std::ostringstream{};

    VTKWriter::write_header(ss, 3, 4);
    const auto expected = std::string{"# vtk DataFile Version 4.1\n"
                                      "vtk output\n"
                                      "ASCII\n"
                                      "DATASET STRUCTURED_POINTS\n"
                                      "DIMENSIONS 3 4 1\n"
                                      "SPACING 1 1 1\n"
                                      "ORIGIN 0 0 0\n"
                                      "POINT_DATA 12\n"
                                      "SCALARS ScalarField double\n"
                                      "LOOKUP_TABLE default\n"};
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(WriteData, WritesDataToStream) {
    auto ss = std::ostringstream{};

    const auto data = Matrix2D{{1, 2}, {3, 4}};
    const auto expected = std::string{"1.00000 2.00000\n3.00000 4.00000\n"};
    VTKWriter::write_data(ss, data);
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(WriteRankData, WritesRankDataToStream) {
    auto ss = std::ostringstream{};

    const auto rank_data = Matrix2D{{0, 0}, {1, 1}};
    const auto expected = std::string{"SCALARS RankId int\nLOOKUP_TABLE default\n0 0\n1 1\n"};
    VTKWriter::write_rank_data(ss, rank_data);
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(WriteFile, WritesAVTKFile) {
    auto ss = std::ostringstream{};

    const auto data = Matrix2D{{1, 2, 3}, {4, 5, 6}};

    const auto expected_header = std::string{"# vtk DataFile Version 4.1\n"
                                             "vtk output\n"
                                             "ASCII\n"
                                             "DATASET STRUCTURED_POINTS\n"
                                             "DIMENSIONS 3 2 1\n"
                                             "SPACING 1 1 1\n"
                                             "ORIGIN 0 0 0\n"
                                             "POINT_DATA 6\n"
                                             "SCALARS ScalarField double\n"
                                             "LOOKUP_TABLE default\n"};
    const auto expected_data = std::string{"1.00000 2.00000 3.00000\n4.00000 5.00000 6.00000\n"};

    const auto expected = expected_header + expected_data;

    VTKWriter::write_file(ss, data);
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(RecordTimestep, CanWriteResultsIntoAFile) {
    const auto data = Matrix2D{{1, 2}, {3, 4}};

    const auto output_folder = std::filesystem::path{"test_CanWriteResultsIntoAFile"};
    const auto files_prefix = "vtk_output_";
    constexpr auto current_iteration = 0;

    const auto writer = VTKWriter{output_folder, files_prefix};


    writer.record_timestep(current_iteration, data, 0);

    const auto expected_path =
            output_folder / (files_prefix + std::to_string(current_iteration) + ".vtk");
    ASSERT_TRUE(std::filesystem::exists(expected_path));


    auto input = std::ifstream{expected_path, std::ios::in | std::ios::binary};
    auto output = std::ostringstream{};
    output << input.rdbuf();

    const auto expected_header = std::string{"# vtk DataFile Version 4.1\n"
                                             "vtk output\n"
                                             "ASCII\n"
                                             "DATASET STRUCTURED_POINTS\n"
                                             "DIMENSIONS 2 2 1\n"
                                             "SPACING 1 1 1\n"
                                             "ORIGIN 0 0 0\n"
                                             "POINT_DATA 4\n"
                                             "SCALARS ScalarField double\n"
                                             "LOOKUP_TABLE default\n"};
    const auto expected_data = std::string{"1.00000 2.00000\n3.00000 4.00000\n"};

    const auto expected = expected_header + expected_data;

    EXPECT_THAT(output.str(), Eq(expected));
    std::filesystem::remove_all(output_folder);
}

TEST(RecordTimestep, CanWriteResultsWithRankInfoIntoAFile) {
    const auto data = Matrix2D{{1, 2}, {3, 4}};
    const auto rank_data = Matrix2D{{0, 0}, {1, 1}};

    const auto output_folder = std::filesystem::path{"test_CanWriteResultsWithRankInfoIntoAFile"};
    const auto files_prefix = "vtk_output_";
    constexpr auto current_iteration = 0;

    const auto writer = VTKWriter{output_folder, files_prefix};

    writer.record_timestep(current_iteration, data, 0, rank_data);

    const auto expected_path =
            output_folder / (files_prefix + std::to_string(current_iteration) + ".vtk");
    ASSERT_TRUE(std::filesystem::exists(expected_path));

    auto input = std::ifstream{expected_path, std::ios::in | std::ios::binary};
    auto output = std::ostringstream{};
    output << input.rdbuf();

    const auto expected_header = std::string{"# vtk DataFile Version 4.1\n"
                                             "vtk output\n"
                                             "ASCII\n"
                                             "DATASET STRUCTURED_POINTS\n"
                                             "DIMENSIONS 2 2 1\n"
                                             "SPACING 1 1 1\n"
                                             "ORIGIN 0 0 0\n"
                                             "POINT_DATA 4\n"
                                             "SCALARS ScalarField double\n"
                                             "LOOKUP_TABLE default\n"};
    const auto expected_data = std::string{"1.00000 2.00000\n3.00000 4.00000\n"};
    const auto expected_rank_data = std::string{"SCALARS RankId int\n"
                                                "LOOKUP_TABLE default\n"
                                                "0 0\n1 1\n"};

    const auto expected = expected_header + expected_data + expected_rank_data;

    EXPECT_THAT(output.str(), Eq(expected));
    std::filesystem::remove_all(output_folder);
}

TEST(RecordTimestep, OnlyRank0RecordsATimeStep) {
    const auto data = Matrix2D{{1, 2}, {3, 4}};

    const auto output_folder = std::filesystem::path{"test_OnlyRank0RecordsATimeStep"};
    const auto files_prefix = "vtk_output_";
    constexpr auto current_iteration = 0;

    const auto writer = VTKWriter{output_folder, files_prefix};


    writer.record_timestep(current_iteration, data, 1);

    const auto expected_path =
            output_folder / (files_prefix + std::to_string(current_iteration) + ".vtk");
    ASSERT_FALSE(std::filesystem::exists(expected_path));
}