# Profiling Demo

This is not exactly a demo, but a dummy code useful when one needs to profile the library. Currently, we take
profiling information via [tracy](https://github.com/wolfpld/tracy?tab=readme-ov-file). If you want to compile this
code, make sure to set the cmake variable `PROFILING` to `ON`.

First, get tracy:

```bash
git clone https://github.com/wolfpld/tracy.git
cd tracy
git checkout <tracy-version>
```

Where `tracy-version` is the version of tracy indicated in our [conanfile](./../../conanfile.txt).

Then, configure and compile the profiler:

```bash
cmake -B profiler/build -S profiler -DCMAKE_BUILD_TYPE=Release
cmake --build profiler/build --config Release --parallel
```

Now you should be able to start the profiler:

```bash
./profiler/build/tracy-profiler
```

Once started, click on `Connect`, which should display a "Waiting for connection" dialog. After this, run this demo with
N ranks:

```bash
mpirun -np N ./profiling_demo.out
```

You should start seeing profiling information in the tracy profiler.