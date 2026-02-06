set(LIBRARY_NAME "heat_transfer")

set(SOURCES
        src/grid_operations.cpp
        src/matrix.cpp
        src/processor_info.cpp
        src/vtk_writer.cpp
)

add_library(${LIBRARY_NAME}
        ${SOURCES}
)

target_link_libraries(${LIBRARY_NAME}
        PRIVATE MPI::MPI_CXX
        PUBLIC reshuffle
)

target_include_directories(${LIBRARY_NAME}
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(${LIBRARY_NAME}
        PUBLIC cxx_std_23
)

add_subdirectory(tests)
add_subdirectory(app)
