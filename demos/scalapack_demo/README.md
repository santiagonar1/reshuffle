# ScaLAPACK demo

Unlike the other demos, this one is a showcase of how things could be implemented if one were to use ScaLAPACK instead
of reshuffle for data distribution. To compile it, make sure to install the scalapack library first in your system and
to set the `RESHUFFLE_TEST_SCALAPACK` cmake variable to `ON` when configuring the project.