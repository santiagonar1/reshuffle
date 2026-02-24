# COSTA

As explained in [their repository](https://github.com/eth-cscs/COSTA/tree/master):

> COSTA is a communication-optimal, highly-optimised algorithm for data redistribution across multiple processors,
> using MPI and OpenMP and offering the possibility to transpose and scale some or all data.

You can check our [costa_benchmark.cpp](../benchmarks/costa_benchmark.cpp) for an example of how to use it. The build
system automatically downloads and builds the library. If you want to do a manual installation, then see below.

## Manual Installation

First take a look at [their installation instructions](https://github.com/eth-cscs/COSTA/blob/master/INSTALL.md).

Once you have followed their instructions, simply indicate the installation path to `cmake` via the
`-DCMAKE_PREFIX_PATH` flag.

```shell
cmake -DCMAKE_PREFIX_PATH=/path/to/costa/installation
```

There are some particularities of macOS that will be discussed next.

### macOS

We will need to install `libomp`, and latter indicate the installation path. We assume that you will do so via
[Homebrew](https://brew.sh/). If that is not the case, adjust the paths passed to `cmake` to the correct location
of the library. We also assume that you already installed [ScaLAPACK](./scalapack.md).

1. Install `libomp`:

```shell
brew install libomp
```

2. Configure, build, and install `COSTA`. We assume that you will install it on a `external` folder in the root of this
   repository. We will activate their wrapper for `ScaLAPACK`.

```shell
cd /path/to/costa/repo
mkdir build && cd build
export SCALAPACK_ROOT=/opt/homebrew/opt/scalapack
cmake .. \
-DCMAKE_INSTALL_PREFIX="/path/to/this/repository/external/costa" \
-DOpenMP_C_FLAGS=-fopenmp=lomp \
-DOpenMP_CXX_FLAGS=-fopenmp=lomp \
-DOpenMP_C_LIB_NAMES="libomp" \
-DOpenMP_CXX_LIB_NAMES="libomp" \
-DOpenMP_libomp_LIBRARY="/opt/homebrew/opt/libomp/lib/libomp.dylib" \
-DOpenMP_CXX_FLAGS="-Xpreprocessor -fopenmp /opt/homebrew/opt/libomp/lib/libomp.dylib -I/opt/homebrew/opt/libomp/include" \
-DOpenMP_CXX_LIB_NAMES="libomp" \
-DOpenMP_C_FLAGS="-Xpreprocessor -fopenmp /opt/homebrew/opt/libomp/lib/libomp.dylib -I/opt/homebrew/opt/libomp/include" \
-DCOSTA_SCALAPACK=CUSTOM
make -j
make install
```
