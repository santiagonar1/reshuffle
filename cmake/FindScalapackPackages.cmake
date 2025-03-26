# Find the required basic packages
find_package(LAPACK REQUIRED)
find_package(BLAS REQUIRED)

# Try pkg-config first for ScaLAPACK
find_package(PkgConfig QUIET)
if (PKG_CONFIG_FOUND)
    pkg_check_modules(SCALAPACK scalapack)
endif ()

# If pkg-config didn't work, try direct path
if (NOT SCALAPACK_FOUND)
    find_library(SCALAPACK_LIBRARIES
            NAMES scalapack scalapack-openmpi
            PATHS
            /usr/lib
            /usr/local/lib
            /opt/homebrew/lib
            $ENV{SCALAPACK_DIR}/lib
    )
endif ()

if (NOT SCALAPACK_LIBRARIES)
    message(FATAL_ERROR "Could not find ScaLAPACK library")
endif ()

# Create an interface library to make it easier to link against all libraries
add_library(LinearAlgebra INTERFACE)
target_link_libraries(LinearAlgebra
        INTERFACE
        ${SCALAPACK_LIBRARIES}
        ${LAPACK_LIBRARIES}
        ${BLAS_LIBRARIES}
)

if (SCALAPACK_INCLUDE_DIRS)
    target_include_directories(LinearAlgebra
            INTERFACE
            ${SCALAPACK_INCLUDE_DIRS}
    )
endif ()