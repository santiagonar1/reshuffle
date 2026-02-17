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

- Clang-19/gcc-14 or newer.
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

### Install

The previous steps are enough to play with the demos and check that dependencies of the library are satisfied. But If
you want to use `reshuffle` with an external project, you will need to install it. We provide
an [installation script](install.sh), which should be run directly from the root directory of the library:

```shell
bash install.sh
```

This should create an `install` directory inside the main directory (i.e., `/path/to/reshuffle/install`). You can use
the variable [CMAKE_INSTALL_PREFIX](https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX.html) to change
this behaviour.

You can check available options via:

```shell
bash install.sh --help
```

Keep in mind that certain things, such as the path to MPI, might need to be modified. So take a look at the script
before
running it. You are encouraged to take a look at the scripts to check the steps required to install the library, which
then you could adapt to your needs.

#### Using reshuffle in external project

In the client code (i.e., the external project where you want to use `reshuffle`) you will need to first find
and link `reshuffle`. For this, in your `CMakeList.txt` add:

```cmake
find_package(reshuffle CONFIG REQUIRED)
target_link_libraries(exec.out PRIVATE reshuffle::reshuffle)
```

When you configure your external application remember to include the path to `reshuffle`:

```shell
cmake -DCMAKE_PREFIX_PATH=/path/to/reshuffle/install
```

You should be able to include and use `reshuffle` in your project as:

```c++
#include <reshuffle/reshuffle.hpp>
```

You can find an example of how this is done in [our demo](https://gitlab.lrz.de/reshuffle/demo).

You should have access to both MPI and zpp_bits in the project you are linking reshuffle with.

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
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DMPIEXEC_PREFLAGS=--allow-run-as-root -DTEST_COSTA=off ..
make
```

### Dyn Proc Demo

**NOTE**: This demo is for now deprecatd, as it uses an old version of the reshuffle API

We have implemented a demo that makes use of a modified OpenMPI library, which allows for expansions and reductions
via PSets and MPI Sessions, described in [this paper](https://dl.acm.org/doi/abs/10.1145/3555819.3555856).
The whole infrastructure describe in the aforementioned paper has been made available by the authors
[here](https://gitlab.inria.fr/dynres).

Once you have set up the infrastructure, you can turn on the compilation of the demo via the `BUILD_DYN_PROC_DEMOS`
option. Additionally, one needs to run programs via `prterun` instead of `mpirun` plus some flags. This is relevant to
us because by default GTest will try to use `mpirun` to run the tests, which will fail on the docker cluster. Therefore,
make sure to indicate the correct values via the `DMPIEXEC_EXECUTABLE` and `MPIEXEC_PREFLAGS` variables. Bellow you can
see an example of the correct configuration for a cluster with 4 nodes (named `n1` to `n4`), each one with 8 cores:

```sh
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DMPIEXEC_EXECUTABLE=prterun -DMPIEXEC_PREFLAGS='--display;map;--mca;btl_tcp_if_include;eth0;--host;n1:8,n2:8,n3:8,n4:8;-x;LD_LIBRARY_PATH' ..
```

## Usage

The easiest way to start using the library, is to take a look at our [demos](./demos) folder. You can also take a look
at our large collection of [tests](./tests) in case you want a more in-depth look at the library and its internals.
