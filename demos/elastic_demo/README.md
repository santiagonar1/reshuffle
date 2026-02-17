# Elastic Demo

This directory contains a basic demonstration of reshuffle's functionality. In contrast to our basic demo, here we
showcase how to use reshuffle to exchange information between ranks that belong to different communicators. This is
quite useful, as it allows us to simulate a malleable/elastic application.

To run with N ranks, execute the following command:

```shell
mpirun -np N ./elastic_demo.out
```

> [!warning]
> You can run this demo with up to five ranks (mostly for printing purposes).
