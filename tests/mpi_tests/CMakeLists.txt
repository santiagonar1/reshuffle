find_package(GTest REQUIRED)
find_package(MPI REQUIRED)

include(GoogleTest)

function(reshuffle_register_mpi_test TEST_BASENAME NUM_CORES)
    set(TEST_NAME "${TEST_BASENAME}.${NUM_CORES}cores")
    # create an executable in which the tests will be stored
    add_executable(${TEST_NAME} ${ARGN} main.cpp)

    target_include_directories(${TEST_NAME}
            PRIVATE include
    )

    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TEST_NAME}
            reshuffle
            GTest::gmock
            GTest::gtest
            MPI::MPI_CXX)

    # Fix taken from: https://gist.github.com/scivision/27a35a3ae69ea59959ac0693447d0764
    if (CMAKE_VERSION VERSION_GREATER_EQUAL 3.29)
        set_property(TARGET ${TEST_NAME} PROPERTY TEST_LAUNCHER ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${NUM_CORES} ${MPIEXEC_PREFLAGS})
    else ()
        set_property(TARGET ${TEST_NAME} PROPERTY CROSSCOMPILING_EMULATOR ${MPIEXEC_EXECUTABLE} ${MPIEXEC_NUMPROC_FLAG} ${NUM_CORES} ${MPIEXEC_PREFLAGS})
    endif ()

    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TEST_NAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER mpi_tests)
endfunction()

reshuffle_register_mpi_test(reshuffle_mpi_tests 2
        dummy_mpi_test.cpp
        inter_communicator_test.cpp
        mpi_comm_utils_test.cpp
        mpi_utils_test.cpp
        shuffle_test.cpp
)
reshuffle_register_mpi_test(reshuffle_reorder_test 6
        reorder_test.cpp
)