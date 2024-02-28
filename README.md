# Reshuffle
A simple MPI library to redistribute data among ranks. The initial goal is to support data redistribution in malleable
applications, where the number of ranks might change at runtime.

## Build
The following should suffice is most scenarios:

```shell
mkdir build && cd build
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake
```

If `cmake` fails to detect the location of your MPI library, use as well:

```
DCMAKE_CXX_COMPILER:FILEPATH=/path/to/mpicxx -DCMAKE_C_COMPILER:FILEPATH=/path/to/mpicc
```