# domoticsController

`domoticsController` is a small C++ application for domotics/home-automation control logic. It loads settings, I/O definitions, badges, and readers from local configuration files and starts the configured reader handlers.

It can be run on any target, as long as every GPIO, wiegandport or serial port is defined in the correct json files.

## Build with CMake

### Requirements

- CMake 3.16+
- A C++17 compiler (for example `g++`)

### Configure and build

```bash
cmake -S . -B build-cmake
cmake --build build-cmake -j
```

The executable is generated at:

- `build-cmake/control`

### Optional CMake targets

Run linter target:

```bash
cmake --build build-cmake --target control-linter
```

## Build with Make

### Requirements

- `make`
- A C++ compiler available as `$(CXX)` (for example `g++`)

### Build

```bash
make
```

The executable is generated at:

- `./control`

### Useful Make targets

Clean build artifacts:

```bash
make clean
```

Run linter target:

```bash
make control-linter
```
