#include <gtest/gtest.h>
#include <mpi.h>


int main(int argc, char **argv) {
    int result = 0;

    testing::InitGoogleTest(&argc, argv);
    // set the gtest death test style to threadsafe
    testing::FLAGS_gtest_death_test_style = "threadsafe";

    MPI_Init(nullptr, nullptr);

    // running only my tests
    result = RUN_ALL_TESTS();

    MPI_Finalize();
    return result;
}