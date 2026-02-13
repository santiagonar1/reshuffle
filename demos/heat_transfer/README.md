# Malleable heat application

This is a showcase of how reshuffle can be used to implement a heat transfer simulation, and also how it can facilitate
making it malleable.

To build this project, simply follow the instructions in the root directory. Once compiled, you can run it with:

```shell
mpirun -n 4 heat_transfer.out --num_iterations 300 --num_rows 100 --num_columns 100 --adaptation_frequency 10 --output_folder output
```

To check all the available options, run:

```shell
./heat_transfer.out --helpfull
```