# Reshuffle Benchmarks

In this directory you will find some benchmark to evaluate the efficiency of the library.
Once the benchmark is compiled, you can run it with the `--benchmark_format=<console|json|csv>`
flag to print the results in the console, in a json file, or in csv. In the case of json or csv,
also use the flag `--benchmark_out=<filename>` to indicate where to store the result (also works for
console). You can also use the `--benchmark_filter=<regex>` to only run a subset of the benchmarks.

Take a look at the [scripts folder](../scripts) where you have some python code to plot the results.

## ScaLAPACK

The [scalapack_benchmark](./scalapack_benchmark.cpp) requires the [ScaLAPACK](https://www.netlib.org/scalapack/) library
to be installed in the system. Refer to the [ScaLAPACK](../docs/scalapack.md) documentation provided in our repository
to see how to install it in your system.

Once that is done, make sure that the option `TEST_SCALAPACK` is turned on.

## COSTA

The [costa benchmark](./costa_benchmark.cpp) requires the [COSTA](https://github.com/eth-cscs/COSTA/tree/master)
library. Refer to the [COSTA](../docs/costa.md) documentation provided in our repository to see how to install it in
your system.

Once that is done, make sure that the option `TEST_COSTA` is turned on.