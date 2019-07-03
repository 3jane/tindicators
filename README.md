# Tulip Indicators

Tulip Indicators is a library of technical analysis indicators. It powers our [Python](./bindings/python) and [Lean](./bindings/lean) libraries of indicators. It was [originally](https://github.com/TulipCharts/tulipindicators) written in C89 and later extended in C++17 here at RCDB.

Please refer to [indicators.yaml](./indicators.yaml) for the comprehensive list of the available indicators. Some extra info regarding formulae sources and future plans is available in the [doc](https://docs.google.com/spreadsheets/d/1WhdTc_AN-_KF_tgcG8B31Tgy6z-pR9rOv1Nr3dxLn5g/edit#gid=444645194).

For any questions please reach out **@ilya.p** on Slack.

## Contributing

See [CONTRIBUTING.md](./CONTRIBUTING.md)

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