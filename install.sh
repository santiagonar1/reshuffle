#!/bin/bash

MPI_LOCATION=/usr/bin
BUILD_DIR=build-reshuffle

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

if [ -d "${BUILD_DIR}" ]; then
  prompt_user
fi

MPI_CC_COMPILER=${MPI_LOCATION}/mpicc
MPI_CXX_COMPILER=${MPI_LOCATION}/mpicxx

rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_CXX_COMPILER:FILEPATH=${MPI_CXX_COMPILER} \
      -DCMAKE_C_COMPILER:FILEPATH=${MPI_CC_COMPILER} -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --target install

cd ..
rm -rf ${BUILD_DIR}
mkdir -p ${BUILD_DIR}
cd ${BUILD_DIR}
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_CXX_COMPILER:FILEPATH=${MPI_CXX_COMPILER} \
      -DCMAKE_C_COMPILER:FILEPATH=${MPI_CC_COMPILER} -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target install
cd ..
rm -rf ${BUILD_DIR}