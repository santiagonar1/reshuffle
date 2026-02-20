# Dyn Proc Demo

> [!warning]
> This demo is for now deprecated, as it is not easy to run it on a local machine nor on the CI

This demo can be seen as a continuation of our simple [elastic demo](../elastic_demo), but now we are not just
simulating an adaptation, but actually performing it. This is powered by the [dynres](https://gitlab.inria.fr/dynres)
library, described in this paper](https://dl.acm.org/doi/abs/10.1145/3555819.3555856). Basically, it uses a modified
version of the [OpenMPI libray](https://www.open-mpi.org/) to allow expansions and reductions in the number of processes
running on a cluster via PSets and MPI Sessions. To successfully run this demo, you first need to set up the
infrastructure for the cluster, and then set the `BUILD_DYN_PROC_DEMOS` option to `ON` when configuring `reshuffle`.

Be also aware that on this environment, one needs to run programs via `prterun` instead of `mpirun`. This is relevant to
us because by default GTest will try to use `mpirun` to run the tests, which will fail on the docker cluster. Therefore,
make sure to indicate the correct values via the `DMPIEXEC_EXECUTABLE` and `MPIEXEC_PREFLAGS` variables. Bellow you can
see an example of the correct configuration for a cluster with 4 nodes (named `n1` to `n4`), each one with 8 cores:

```sh
cmake -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake \
      -DMPIEXEC_EXECUTABLE=prterun \
      -DMPIEXEC_PREFLAGS='--display;map;--mca;btl_tcp_if_include;eth0;--host;n1:8,n2:8,n3:8,n4:8;-x;LD_LIBRARY_PATH' \
      -DRESHUFFLE_BUILD_DYN_PROC_DEMO=ON ..
```