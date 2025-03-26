# Find the required basic packages
find_package(LAPACK REQUIRED)
find_package(BLAS REQUIRED)

find_package(costa REQUIRED HINTS ${CMAKE_CURRENT_SOURCE_DIR}/external)

# Create an interface library to make it easier to link against all libraries
add_library(CostaLibrary INTERFACE)
target_link_libraries(CostaLibrary
        INTERFACE
        costa::costa_scalapack
        costa::costa
        ${LAPACK_LIBRARIES}
        ${BLAS_LIBRARIES}
)

if (COSTA_INCLUDE_DIRS)
    target_include_directories(CostaLibrary
            INTERFACE
            ${COSTA_INCLUDE_DIRS}
    )
endif ()