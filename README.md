# Reshuffle

A simple MPI library to redistribute data among ranks. The initial goal is to support data redistribution in elastic
applications, where the number of ranks might change at runtime.

**NOTE**: Keep in mind that the library is currently under development, and thus the API is not yet stable. This also
means that we still need to improve the performance of the library.

[TOC]

## Build and Install

If you are trying to do more complicated things, like building the library with different compilers in the same
machine, please take a look at [compiling rehsuffle](./docs/compiling_reshuffle.md).

### Requirements

We like to use bleeding edge compilers, as they are more likely to catch bugs and allows us to experiment with new
features of the language:

- Clang-21/gcc-15 or newer.
- [Conan 2](https://docs.conan.io/2/installation.html).
- ScaLAPACK (if you want to compile the benchmark).
- [Costa](https://github.com/eth-cscs/COSTA/tree/master) (if you want to compile the benchmark).

### Build

We use [conan](https://conan.io/) as package manager. Thus, in order to correctly install all the dependencies
required by the library, you need to specify a `conan_provider.cmake` file. We include one in our project, so
the following lines should be enough to compile the library and its examples.

```shell
mkdir build && cd build
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake ..
make
```

If `cmake` fails to detect the location of your MPI library, use instead:

```sh
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_CXX_COMPILER:FILEPATH=/path/to/mpicxx -DCMAKE_C_COMPILER:FILEPATH=/path/to/mpicc ..
```

There are additional options to, for example, disable the compilation of the test files. Check those directly in the
cmake configuration.

If you happen to use MacOS, we highly recommend using [Homebrew](https://brew.sh/) to install Clang. If you do so,
use the [MacBrewLLVMToolchain](cmake/MacBrewLLVMToolchain.cmake) to configure your environment:

```shell
mkdir build && cd build
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_TOOLCHAIN_FILE=cmake/MacBrewLLVMToolchain.cmake ..
make
```

### Install / Use in external projects

The previous steps are enough to play with the demos and check that dependencies of the library are satisfied. But If
you want to use `reshuffle` with an external project, you will need to install it. If your project is a CMake project,
we suggest you take a look at our [Simple reshuffle app](https://github.com/santiagonar1/simple-reshuffle-app) project,
where we showcase how to easily install `reshuffle` in an external project.

You can also manually install `reshuffle` in your system. For this, use the `-DCMAKE_INSTALL_PREFIX` at configuration
time to indicate where to install the library. After this, simply run `make install` (or `ninja install` if you are
using [Ninja](https://ninja-build.org/)).

There is an [installation script](install.sh), but this is  **experimental** and might not work for all cases.

### Docker

You can use the `Dockerfile` provided by us to create an image in which to build and run the code. So first, build
the image:

```shell
docker build -t reshuffle .
```

Then you can start it and compile/run the code. Note that since the image uses OpenMPI, we need to allow the execution
of MPI code as root in order to successfully compile the tests. This is done with the `--allow-run-as-root`, set via
the `MPIEXEC_PREFLAGS` variable.

```shell
docker run --rm -it -v .:/reshuffle reshuffle bash
mkdir /reshuffle/build && cd /reshuffle/build
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DMPIEXEC_PREFLAGS=--allow-run-as-root ..
make
```

## Usage

The easiest way to start using the library, is to take a look at our [demos](./demos) folder. You can also take a look
at our large collection of [tests](./tests) in case you want a more in-depth look at the library and its internals.
