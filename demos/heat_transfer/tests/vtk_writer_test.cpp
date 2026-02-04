#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <vtk_writer.hpp>

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