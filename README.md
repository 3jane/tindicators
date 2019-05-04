# Tulip Indicators

Tulip Indicators is a library of technical analysis indicators.

It was [originally](https://github.com/TulipCharts/tulipindicators) written in C89 and extended in C++17 by us later.

Please refer to [indicators.yaml](./indicators.yaml) for the comprehensive list of the available indicators. Some extra info regarding formulae sources and future plans is available in the [doc](https://docs.google.com/spreadsheets/d/1WhdTc_AN-_KF_tgcG8B31Tgy6z-pR9rOv1Nr3dxLn5g).

## Bindings

There are some nice bindings available, for use in [Python](https://github.com/hcmc-project/tulipindicators-python) and [Lean](https://github.com/hcmc-project/tulipindicators-net).

## Testing

The testing process consists of three stages:
1. **`benchmark2`**: a series of 4000 bars is randomly generated, then the behavior of different implementations (plain, `ref`, `stream`) is matched against each other, and also benchmarked.
2. **`fuzzer`**: let's try to find options that would trigger a segfault, memleak, of something alike.
3. **`smoke`**: we match the behavior of the indicator against precomputed values.

Built on Linux with `-DCMAKE_BUILD_TYPE=Debug`, these are run under sanitizers, namely `-fsanitize=undefined -fsanitize=address -fsanitize=leak`.

## Overview

1. **indicators.yaml**: the comprehensive index of the indicators present in the library.
1. **codegen.py**: generates, based on `indicators.yaml`,
    + the boilerplate under `indicators/` for further indicator implementation
    + `indicators.h`
    + `indicators_index.c`
2. **indicators/\***: `xxx.c` contains code implementing xxx, namely:
    + `int xxx(int size, double **inputs, double *options, double **outputs)`
    + `int xxx_start(double *options)`  
    + `int xxx_stream_new(double *options, ti_stream **stream)`
    + `int xxx_stream_run(ti_stream *stream, int size, double **inputs, double **outputs)`
    + `void xxx_stream_free(ti_stream *stream)`   
    + `int xxx_ref(int size, double **inputs, double *options, double **outputs)`  
3. **utils/\***: some data structures and macros that come up frequently, namely
    + `buffer.h`: simple ringbuffer
    + `localbuffer.h`: ringbuffer allocated locally to the rest of the data
    + `log.h`: contains `LOG()` macro, useful for debugging
    + `minmax.h`: contains `MIN()`, `MAX()` macros
    + `testing.h`: some code shared by testing utilities, like `compare_answers()`
4. **tests/\***: contains precomputed tests, namely
    + `atoz.txt`: cases from *Technical Analysis from A to Z*
    + `extra.txt`: cases from other sources
    + `untest.txt`: regression tests
4. **indicators.h**: the ultimate library header file.
5. **indicators_index.c**: the runtime table of the indicators.

## Building

Install the dependencies:

```
- python3, pyyaml
- cmake
- some reasonably modern C++ compiler
```

Build as a regular CMake project:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j
```

You should get a shared library `libindicators.so` as a result (the exact name depends on the platform) and the test binaries. You may want to run `./benchmark2` to see the performance of the indicators of your interest.

## Contributing

1. find a reliable source like a book or a publication at [traders.com](https://traders.com)
1. create a branch according to our [conventions](https://github.com/hcmc-project/docs/blob/master/git.md)
1. add an entry to `indicators.yaml` and run `codegen.py`
2. go to `indicators/xxx.cc` and implement the indicator
3. for the idiomatic constructs, you may want to look at some recently added indicators
4. consider adding a precomputed testcase to `tests/extra.txt` if the author provides one
4. make sure your implementation passes `ctest` in debug mode
5. squash your commits into one
5. create a PR, specify the source, and attach a screenshot of the definition - see older [PRs](https://github.com/hcmc-project/tulipindicators-private/pull/6) for an example
