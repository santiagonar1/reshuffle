find_package(GTest REQUIRED)
find_package(MPI REQUIRED)

include(GoogleTest)

macro(package_add_mpi_test TEST_NAME)
    # create an executable in which the tests will be stored
    add_executable(${TEST_NAME} ${ARGN})

    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TEST_NAME}
            reshuffle
            GTest::gmock
            GTest::gtest
            MPI::MPI_CXX)

    set(RESHUFFLE_TESTS_NUM_MPI_RANKS 2 CACHE STRING "Number of MPI Ranks to run Reshuffle tests with.")
    set_property(TARGET ${TEST_NAME} PROPERTY CROSSCOMPILING_EMULATOR '${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${RESHUFFLE_TESTS_NUM_MPI_RANKS} ${MPIEXEC_PREFLAGS}')

    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TEST_NAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_DIR}"
    )
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER tests)
endmacro()

file(GLOB SOURCES
        *.cpp
)

package_add_mpi_test(reshuffle_mpi_tests
        ${SOURCES})