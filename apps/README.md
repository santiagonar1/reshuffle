# Reshuffle Demos

In this folder you find a collection of demos that showcase the use of the library.

## `demo`

A simple code that splits a 2D matrix among 4 ranks using different strategies (e.g., 4X1, 2x2, 1x4). You need to
run the code with 4 ranks, as follows:

```sh
mpirun -np 4 demo.out
```

## `elastic_demo`

This code simulates a scenario of adding one rank to the simulation until `n`, where `n` is the number of ranks you
run the code with. The library takes care of scattering a buffer among the active ranks, and prints the contents
of rank 0. As more ranks join, you should see how the size of this buffer decreases. To run it, simply do:

```sh
mpirun -np n elastic_demo.out
```

## `dyn_proc_demo`

Check our [README](../README.md) to see how to enable the compilation of this example. Once you have done that, and
you have configured the docker instances, you simply need to do:

```sh
prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n1:8,n2:8,n3:8,n4:8  -x LD_LIBRARY_PATH ./dyn_proc_demo.out
```

The above code will execute an expansion. If you want to try a reduction, simply indicate the `--reduction` flag, as
follows:

```sh
prterun -np 8 --display map --mca btl_tcp_if_include eth0 --host n1:8,n2:8,n3:8,n4:8  -x LD_LIBRARY_PATH ./dyn_proc_demo.out --reduction
```
