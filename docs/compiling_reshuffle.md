# Compiling Reshuffle

## How to use different compilers

When conan is installed, a default profile is created (usually located in `~/.conan2/profiles/default`), which will be
used to compile the libraries listed in the `conanfile.txt`. This works well if you are planning to use the same
compiler listed in the default profile to build reshuffle, but it might cause problems if that is not the case. The
classic example is trying to build reshuffle with GCC on Mac, where most likely the default conan profile is using
an LLVM compiler (i.e., Apple clang, Clang); this is problematic, as those two are ABI incompatible.

Thus, please make sure that you use in your conan profile the same compiler you use to build reshuffle. Furthermore,
it is better to also match the build type (e.g., Release). The easiest approach is to create different profiles
(e.g., `gcc-release`, `gcc-debug`), and set them via the `-DCONAN_HOST_PROFILE` and `-DCONAN_BUILD_PROFILE` cmake
flags.

For example, to build with GCC you might use:

```shell
cmake .. -DCMAKE_PROJECT_TOP_LEVEL_INCLUDES=conan_provider.cmake -DCONAN_HOST_PROFILE=/path/to/gcc/profile \
         -DCONAN_BUILD_PROFILE=/path/to/gcc/profile
```

