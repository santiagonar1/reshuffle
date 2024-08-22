# Reshuffle

A simple MPI library to redistribute data among ranks. The initial goal is to support data redistribution in elastic
applications, where the number of ranks might change at runtime.

**NOTE**: Keep in mind that the library is currently under development, and thus the API is not yet stable. This also
means that we still need to improve the performance of the library.

[TOC]

## Build and Install

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

Also, please include in your `conanfile.txt` `zpp_bits/4.4.20` as requirement. This is a temporal limitation that we
hope to remove in future versions.

Finally, when you configure your external application do not forget to include the path to `reshuffle`:

```shell
cmake -DCMAKE_PREFIX_PATH=/path/to/reshuffle/install
```

At last, you should be able to include and use `reshuffle` in your project as:

```c++
#include <reshuffle/reshuffle.hpp>
```

You can find an example of how this is done in [our demo](https://gitlab.lrz.de/reshuffle/demo).

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

### Dyn Proc Demo

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

### TLDR

Currently, supports 1D and 2D buffers. Below you see an example of partitioning a 2D domain:

```c++
constexpr int num_values_x = 20;
constexpr int num_valuex_y = 20;
constexpr int num_values = num_rows * num_columns;

// Let's assume that all values are in rank 0
const auto old_distribution = std::array{reshuffle::make_block_wise(num_values_x, 1),
                                         reshuffle::make_block_wise(num_valuex_y, 1)};

const auto new_distribution = std::array{reshuffle::make_block_wise(num_values_x, 4),
                                         reshuffle::make_block_wise(num_valuex_y, 1)};

matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, old_distribution, new_distribution);
```

You can also move the data from a set of ranks specified in a `MPI_Comm` to a set of ranks in a different
communicator. The only current restriction is that rank 0 has to be present in both. For example, we could
redistribute data to a subset of ranks with:

```c++
// Asume comm_subsert is an MPI_Comm with a subset of ranks
const auto new_values = reshuffle::shuffle(values, MPI_COMM_WORLD, comm_subset);
```

Since the project is still under heaver development, the easiest way to see how the library can be used is to
check our [demos](demos) and the [shuffle tests](tests/mpi_tests/shuffle_test.cpp).

### Basics

You can find detailed examples of the library functionality inside the [tests](tests) folder, and demo applications
inside [demos](demos).

The main function in the library is `shuffle`, which takes care of splitting some data among ranks. For example, the
following line will take the data stored in `buffer` on each rank, merge it, and split it again between them.

```c++
buffer = reshuffle::shuffle(buffer, MPI_COMM_WORLD)
```

It is possible to partition a domain in different ways specifying a data distribution. For example, the line below
goes from a block wise distribution with 2 blocks, to 4 blocks.

```c++
const auto old_distribution = reshuffle::make_block_wise(num_values, 2);
const auto new_distribution = reshuffle::make_block_wise(num_values, 4);

buffer = reshuffle::shuffle(buffer, MPI_COMM_WORLD, old_distribution, new_distribution)
```

`Suffle` should work out of the box with any iterable container (even non-consecutive, as `std::list`) and with any
fundamental datatype supported by MPI (e.g., `int`), as well as any aggregate datatype. For more complex types,
the user needs to use [zpp_bits](https://github.com/eyalz800/zpp_bits) to serialize the type.

### 2D Datatypes

We are working on supporting multidimensional datatypes, but for now only 2D types (e.g., `std::vector<std::vector<>>`)
are supported. If you work in 2D, you must include the old and new data distributions. The reason for this
is that, unlike in 1D, there is not a clear default of how to partition the domain.

### Data Distributions

We provide a `BlockCyclic` data distribution which, as the name indicates, takes a certain number of values, number of
blocks, and block size, and use it to split a domain in blocks, assigning them in a round robin fashion to each rank in
increasing rank id order. We provide the `make_block_wise` function to make a BlockWise division.

### Using different communicators

An alternative version of `shuffle` allows you to indicate different mpi communicators for the origin and destination of
the data. The only current restriction is that rank 0 must belong to both communicators. This can be used to
redistribute data before an adaptation occurs (or after, in case of an expansion).