# ScaLAPACK

The [ScaLAPACK](https://www.netlib.org/scalapack/) library is a collection of high-performance linear algebra routines
for parallel distributed memory machines. Among the many functions it provides, it includes the `pdgemr2d_` to
distribute 2D matrices that store doubles.

Check our [scalapack_demo.cpp](../demos/scalapack_demo.cpp) for an example of how that function can be used. For
variants for other datatypes beside doubles,
check [the declarations in the COSTA repository](https://github.com/eth-cscs/COSTA/blob/master/utils/pxgemr2d_utils.hpp).

## Installation

### macOS

It can be installed via [Homebrew](https://brew.sh/):

```shell
brew install scalapack
```

This should install the library in `/opt/homebrew/opt/scalapack`