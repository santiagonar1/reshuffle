#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <mpi.h>
#include <utils.hpp>

using reshuffle::internal::to_mpi_datatype;
using ::testing::Eq;

TEST(ToMPIDatatype, ConvertsDatatypeToMPIDatatype) {
    EXPECT_THAT(to_mpi_datatype<int>(), Eq(MPI_INT));
    EXPECT_THAT(to_mpi_datatype<float>(), Eq(MPI_FLOAT));
    EXPECT_THAT(to_mpi_datatype<double>(), Eq(MPI_DOUBLE));
}

TEST(ToMPIDatatype, ThrowsIfDatatypeCannotBeConverted) {
    EXPECT_THROW(to_mpi_datatype<char>(), std::invalid_argument);
}