#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vtk_writer.hpp>

#include <fstream>

using namespace heat::vtk;
using namespace heat;

using testing::Eq;

TEST(WriteHeader, WritesHeaderVTKFile) {
    auto ss = std::ostringstream{};

    write_header(ss, 3, 4);
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
    write_data(ss, data);
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(WriteFile, WritesAVTKFile) {
    auto ss = std::ostringstream{};

    const auto data = Matrix2D{{1, 2}, {3, 4}};

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

    write_file(ss, data);
    EXPECT_THAT(ss.str(), Eq(expected));
}

TEST(WriteFile, CanWriteResultsIntoAFile) {
    const auto data = Matrix2D{{1, 2}, {3, 4}};
    constexpr auto test_file = std::string{"test.vtk"};

    write_file(test_file, data);

    auto input = std::ifstream{test_file, std::ios::in | std::ios::binary};
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

    std::remove(test_file.c_str());
}