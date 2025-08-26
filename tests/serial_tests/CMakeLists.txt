find_package(GTest REQUIRED)
include(GoogleTest)

function(package_add_test TEST_NAME)
    # create an executable in which the tests will be stored
    add_executable(${TEST_NAME} ${ARGN})

    target_include_directories(${TEST_NAME}
            PRIVATE include
    )

    # link the Google test infrastructure, mocking library, and a default main function to
    # the test executable.  Remove g_test_main if writing your own main function.
    target_link_libraries(${TEST_NAME}
            PRIVATE GTest::gtest
            PRIVATE GTest::gtest_main
            PRIVATE reshuffle)

    # gtest_discover_tests replaces gtest_add_tests,
    # see https://cmake.org/cmake/help/v3.10/module/GoogleTest.html for more options to pass to it
    gtest_discover_tests(${TEST_NAME}
            # set a working directory so your project root so that you can find test data via paths relative to the project root
            WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
            PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
    )
    set_target_properties(${TEST_NAME} PROPERTIES FOLDER serial_tests)
endfunction()

file(GLOB SOURCES
        *.cpp
)

package_add_test(reshuffle_tests
        ${SOURCES})