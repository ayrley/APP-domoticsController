# domoticsController

`domoticsController` is a C++17 home automation controller. It loads settings, GPIO/network I/O, readers, badges, and actions from JSON configuration and then runs continuously.

Core features:

- Local reader support (Wiegand)
- Network reader support over TCP
- Badge-based access checks
- Granted/denied action execution
- NanoGUI-based runtime UI

## Runtime architecture

At startup, the controller loads:

- IO definitions
- Badge files
- Settings (user + factory)
- Readers and actions

Readers run on dedicated threads and trigger actions when badge access is granted or denied.

## Project layout

Important locations:

- `src/control.cpp`: Application entrypoint and startup sequence
- `src/settings.cpp`: Settings parsing and object construction
- `src/Controller/reader.cpp`: Reader business logic
- `src/Controller/tcpServer.cpp`: Reusable TCP transport server
- `src/Controller/badgeProtocol.cpp`: Badge payload parsing and response formatting
- `src/GUI/`: UI components
- `factory.settings.domotics`: Factory defaults
- `user.settings.domotics`: Runtime/user overrides

## Configuration

Main settings file structure:

```json
{
	"type": "USER",
	"version": 1,
	"network": {
		"dhcp": true,
		"dns1": "",
		"dns2": "",
		"gateway": "",
		"ipAddress": "",
		"netmask": ""
	},
	"readers": [],
	"actions": []
}
```

### Reader types

- `location_type: "LOCAL"`: Reader connected to local hardware
- `location_type: "IP"`: Reader served over TCP

- `type: "WIEGAND"`: Wiegand reader
- `type: "OSDP"`: Reserved placeholder in current codebase

### Network reader location format

For `location_type: "IP"`, `location` must be one of:

- `"<port>"` (binds to `0.0.0.0:<port>`)
- `"<ipv4>:<port>"` (for example `"127.0.0.1:9000"`)

Example reader config:

```json
{
	"name": "net-reader-1",
	"location": "0.0.0.0:9000",
	"location_type": "IP",
	"type": "WIEGAND",
	"granted": {
		"name": "AccessGranted",
		"outputs": [
			{
				"output": "out_green",
				"duration": 1000
			}
		]
	},
	"denied": {
		"name": "AccessDenied",
		"outputs": [
			{
				"output": "out_red",
				"duration": 1000
			}
		]
	}
}
```

## TCP badge protocol

When a network reader is configured, the controller listens on the configured TCP socket.

Each request should contain one badge payload. The response is a JSON object containing:

- `valid`: `true` when badge is known
- `badge`: parsed badge number (or `0` if parse failed)
- `action`: executed action name (`granted` or `denied` action)
- `error`: included only on invalid payload

### Supported request formats

Plain numeric payload:

```text
123456
```

JSON payload with `badge`:

```json
{"badge":123456}
```

JSON payload with `badgeNumber`:

```json
{"badgeNumber":"123456"}
```

### Example responses

Valid badge:

```json
{"valid":true,"badge":123456,"action":"AccessGranted"}
```

Invalid badge:

```json
{"valid":false,"badge":123456,"action":"AccessDenied"}
```

Invalid payload:

```json
{"valid":false,"badge":0,"action":"AccessDenied","error":"invalid badge payload"}
```

### Quick test with netcat

Send plain badge:

```bash
printf '123456\n' | nc 127.0.0.1 9000
```

Send JSON badge:

```bash
printf '{"badge":123456}\n' | nc 127.0.0.1 9000
```

## GUI

The application includes a NanoGUI dashboard.

Backend selection is runtime-based:

- If `DISPLAY` or `WAYLAND_DISPLAY` is present, it uses desktop window mode
- Otherwise it attempts GLFW null platform mode (framebuffer/headless style)

## Build with CMake

### Requirements

- CMake 3.16+
- C++17 compiler
- NanoGUI available at the path in `NANOGUI_ROOT` (default in `CMakeLists.txt`)

### Configure and build

```bash
cmake -S . -B build-cmake
cmake --build build-cmake -j
```

Helper script:

```bash
./build_cmake.sh
```

With extra CMake arguments:

```bash
./build_cmake.sh -DCMAKE_BUILD_TYPE=Release
```

Binary path:

- `build-cmake/control`

Linter target:

```bash
cmake --build build-cmake --target control-linter
```

## Build with Make

### Requirements

- `make`
- C++ compiler available via `$(CXX)`

### Build

```bash
make
```

Binary path:

- `./control`

Useful targets:

```bash
make clean
make control-linter
```

## Notes

- Badge validation compares incoming badge number with configured `badgeNumber` entries from badge files.
- Network handling is split for reuse:
	- `TcpServer`: transport
	- `BadgeProtocol`: payload and reply protocol
	- `Reader`: access decision and action dispatch
