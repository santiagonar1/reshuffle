# Reshuffle

A simple MPI library to redistribute data among ranks. The initial goal is to support data redistribution in elastic
applications, where the number of ranks might change at runtime.

**NOTE**: Keep in mind that the library is currently under development, and thus the API is not yet stable. This also
means that we still need to improve the performance of the library.

[TOC]

## Build

We use [conan](https://conan.io/) as package manager. Thus, in order to correctly install all the dependencies
required by the library, you need to specify a `conan_provider.cmake` file. We include one in our project, so
the following two lines should be enough to compile the library and its examples.

```shell
mkdir build && cd build
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake ..
make
```

If `cmake` fails to detect the location of your MPI library, use instead:

```sh
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCMAKE_CXX_COMPILER:FILEPATH=/path/to/mpicxx -DCMAKE_C_COMPILER:FILEPATH=/path/to/mpicc ..
```

If you are planning to test the code in docker, then make sure to set the corresponding `MPIEXEC_PREFLAGS`. For example,
if you are using OpenMPI you will need to indicate the `--allow-run-as-root` to be able to compile the tests
(i.e., the tests are immediately run):

```sh
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DMPIEXEC_PREFLAGS=--allow-run-as-root ..
```

## Usage

### TLDR

Currently supports 1D and 2D buffers. Below you see an example of partitioning a 2D domain:

```c++
constexpr int num_rows = 20;
constexpr int num_columns = 20;
constexpr int num_elements = num_rows * num_columns;
constexpr auto global_dimension = reshuffle::Dimensions2D{num_rows, num_columns};

const auto strategies = std::array{reshuffle::BlockWise(4), reshuffle::BlockWise(1)};
auto global_coloring = std::vector<reshuffle::rank_id>(num_elements, 0);
auto local_coloring = std::vector<reshuffle::rank_id>{};

std::tie(global_coloring, local_coloring) =
        reshuffle::create_coloring(global_coloring, global_dimension, strategy, rank).as_tuple();
const auto subdomain_dimension =
        reshuffle::get_subdomain_dimension(strategy, global_dimension, rank);
matrix = reshuffle::shuffle(matrix, MPI_COMM_WORLD, local_coloring, subdomain_dimension);
```

You can also move the data from a set of ranks specified in a `MPI_Comm` to a set of ranks in a different
communicator. The only current restriction is that rank 0 has to be present in both. The example below splits a 1D
buffer from a communicator that only includes rank 0, to MPI_WORLD (i.e., it is effectively a scatter):

```c++
const auto new_values = reshuffle::shuffle(values_only_in_root, comm_rank_0, MPI_COMM_WORLD);
```

### Basics

You can find detailed examples of the library functionality inside the [tests](tests) folder, and demo applications
inside [apps](apps).

The main function in the library is `shuffle`, which takes care of splitting some data among ranks. For example, the
following line will take the data stored in `buffer` on each rank, merge it, and split it again between them.

```c++
buffer = reshuffle::shuffle(buffer, MPI_COMM_WORLD)
```

One can also explicitly indicate to which rank a value should be stored via *coloring*, as seen below:

```c++
// coloring is relative to the data contained in each rank
buffer = reshuffle::shuffle(buffer, MPI_COMM_WORLD, coloring)
```

Right now, for 1D, the library works for both consecutive (e.g., `std::vector`) and non-consecutive (e.g., `std::list`)
containers. It works also out of the box if the values stored in the container are fundamental datatypes (e.g., `int`,
`double`, etc.), or aggregate types. For more complex types, _Reshuffle_ requires the use of
[zpp_bits](https://github.com/eyalz800/zpp_bits) library for their serialization (note that this library is also used
under the hood to automatically serialize aggregate types).

### 2D Datatypes

We are working on supporting multidimensional datatypes, but for now only 2D (e.g., `std::vector<std::vector<>>) are
supported. If you work with a 2D type, you must indicate the coloring (which is optional in 1D). The reason for this is
that, unlike 1D, there is not a clear default of how to partition the domain. You also have to additionally indicate
the expected subdomain size after the shuffling has occurred (check the *Coloring* section to see how to obtain this):

```c++
m = reshuffle::shuffle(m, MPI_COMM_WORLD, coloring, subdomain_dimension);
```

### Coloring

We provide two helper functions for coloring:

1. `create_coloring`.
2. `get_subdomain_dimension`

The first one, `create_coloring`, can be used to get the required coloring to split either a 1D or 2D domain according
to a strategy.

The following example returns the global and local coloring for rank 0 in order to parition the buffer in two blocks:

```c++
const auto strategy = reshuffle::BlockWise(2);// i.e., split the domain in two blocks
const auto [global_coloring, coloring_rank_0] =
        reshuffle::create_coloring(current_coloring, strategy, 0)
```

The following example partitions a 2D matrix of size 20x20 in 2x2 blocks (i.e., 4 ranks):

```c++
const auto strategy_x = reshuffle::BlockWise(2);
const auto strategy_y = reshuffle::BlockWise(2);

const auto strategies = std::array{strategy_x, strategy_y};
const auto global_dimensions = reshuffle::Dimensions2D{20, 20};

const auto [global_coloring, coloring_0] =
        reshuffle::create_coloring(current_coloring, global_dimensions, strategies, 0);
```

The second function, `get_subdomain_dimension`, is used to get the dimension of the subdomain assigned to a specific
rank.

```c++
const auto strategy_x = reshuffle::BlockWise(2);
const auto strategy_y = reshuffle::BlockWise(1);

const auto strategies = std::array{strategy_x, strategy_y};
const auto global_dimensions = reshuffle::Dimensions2D{20, 20};

const auto dimensions_0 = reshuffle::get_subdomain_dimension(strategies, global_dimensions, 0);
```

So, the expected workflow is:

1. Get local and global coloring from `create_coloring`.
2. For 2D: Get the subdomain size via `get_subdomain_dimension`.
3. Use the local coloring and, if partitioning 2D, the subdomain size to shuffle data.
4. Use the global coloring to split the domain again, if needed.

### Using different communicators

An alternative version of `shuffle` allows you to indicate different mpi communicators for the origin and destination of
the data. The only current restriction is that rank 0 must belong to both communicators. This can be used to
redistribute data before an adaptation occurs (or after, in case of an expansion).