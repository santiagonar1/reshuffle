#!/bin/bash

MPI_LOCATION=""
C_COMPILER=""
CXX_COMPILER=""
INSTALL_PREFIX=""

BUILD_DIR="./build-script-$(date +%Y-%m-%d-%H%M%S)"

prompt_help() {
    echo "Usage: $0 [--install-to <path>] [--mpi-dir <path>] [--c-compiler <path>] [--cxx-compiler <path>] [--help]"
    echo ""
    echo "Optional arguments:"
    echo "  --install-to <path>     Specify the installation prefix (e.g., ~/Downloads/install)"
    echo "  --mpi-dir <path>        Specify the MPI installation directory (e.g., /usr/bin)"
    echo "  --c-compiler <path>     Configures the project with the specified C compiler"
    echo "  --cxx-compiler <path>   Configures the project with the specified CXX compiler"
    echo "  --help                  Display this help message and exit"
}

# Function to prompt the user for yes or no
prompt_user() {
    read -p "The folder ${BUILD_DIR} already exists and WILL BE DELETED. Do you want to continue? (yes/no): " yn
    case $yn in
        [Yy]* )
              echo "Proceeding..."
              return
              ;;
          * )
              echo "Aborting..."
              exit 1
              ;;
    esac
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
         --help)
             prompt_help
             exit 0
             ;;
        --install-to)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        --mpi-dir)
            MPI_LOCATION="$2"
            shift 2
            ;;
        --c-compiler)
            C_COMPILER="$2"
            shift 2
            ;;
        --cxx-compiler)
            CXX_COMPILER="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            prompt_help
            exit 1
            ;;
    esac
done

if [ -d "${BUILD_DIR}" ]; then
  prompt_user
fi

MPI_CMAKE_FLAGS=""
if [ ! -z "${MPI_LOCATION}" ]; then
    MPI_CC_COMPILER=${MPI_LOCATION}/mpicc
    MPI_CXX_COMPILER=${MPI_LOCATION}/mpicxx
    MPI_CMAKE_FLAGS="-DCMAKE_CXX_COMPILER:FILEPATH=${MPI_CXX_COMPILER} -DCMAKE_C_COMPILER:FILEPATH=${MPI_CC_COMPILER}"
fi

C_CMAKE_FLAG=""
if [ ! -z "${C_COMPILER}" ]; then
    C_CMAKE_FLAG="-DCMAKE_C_COMPILER=${C_COMPILER}"
fi

CXX_CMAKE_FLAG=""
if [ ! -z "${CXX_COMPILER}" ]; then
    CXX_CMAKE_FLAG="-DCMAKE_CXX_COMPILER=${CXX_COMPILER}"
fi

INSTALL_PREFIX_CMAKE_FLAG=""
if [ ! -z "${INSTALL_PREFIX}" ]; then
    INSTALL_PREFIX_CMAKE_FLAG="-DCMAKE_INSTALL_PREFIX=${INSTALL_PREFIX}"
fi

CMAKE_FLAGS="-DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake \
             ${MPI_CMAKE_FLAGS} ${C_CMAKE_FLAG} ${CXX_CMAKE_FLAG} ${INSTALL_PREFIX_CMAKE_FLAG} \
             -DBUILD_TESTING=OFF"

rm -rf "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE=Debug ${CMAKE_FLAGS} -S . -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target install --parallel

rm -rf "${BUILD_DIR}"
cmake -DCMAKE_BUILD_TYPE=Release ${CMAKE_FLAGS} -S . -B "${BUILD_DIR}"
cmake --build "${BUILD_DIR}" --target install --parallel

rm -rf "${BUILD_DIR}"