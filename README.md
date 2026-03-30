# domoticsController

`domoticsController` is a small C++ application for domotics/home-automation control logic. It loads settings, I/O definitions, badges, and readers from local configuration files and starts the configured reader handlers.

It can be run on any target, as long as every GPIO, wiegandport or serial port is defined in the correct json files.

## GUI

The application includes a NanoGUI-based dashboard that displays:

- Current controller status
- CPU usage
- RAM usage

The GUI backend is selected at runtime:

- If a display server is available (`DISPLAY` or `WAYLAND_DISPLAY`), it uses a normal desktop window.
- If no display server is detected, it attempts to use the GLFW null platform (framebuffer/headless style).

GUI sources are located under:

- `src/screen.cpp`
- `src/GUI/`

## Build with CMake

### Requirements

- CMake 3.16+
- A C++17 compiler (for example `g++`)

### Configure and build

```bash
cmake -S . -B build-cmake
cmake --build build-cmake -j
```

### Build helper script

A helper script is provided to configure and build in one command:

```bash
./build_cmake.sh
```

You can pass extra CMake configure options to the script:

```bash
./build_cmake.sh -DCMAKE_BUILD_TYPE=Release
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

The Make build also compiles GUI sources from `src/GUI/*.cpp`.

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
