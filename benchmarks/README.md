# Reshuffle Benchmarks

In this directory you will find some benchmark to evaluate the efficiency of the library.
Once the benchmark is compiled, you can run it with the `--benchmark_format=<console|json|csv>`
flag to print the results in the console, in a json file, or in csv. In the case of json or csv,
also use the flag `--benchmark_out=<filename>` to indicate where to store the result (also works for
console). You can also use the `--benchmark_filter=<regex>` to only run a subset of the benchmarks.

Take a look at the [scripts folder](../scripts) where you have some python code to plot the results.