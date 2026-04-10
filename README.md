# domoticsController

`domoticsController` is a C++17 embedded access-control and home automation controller. It loads GPIO and network IO definitions, badge files, reader definitions, and input-triggered actions from JSON configuration, then presents a NanoGUI-based local UI.

Core features:

- Local Wiegand readers over device files
- Network readers over TCP
- Remote GPIO inputs and outputs over TCP via `LIB-remote`
- Badge-based access checks
- Input-triggered actions for outputs
- NanoGUI touchscreen or framebuffer UI
- Status LEDs, life LED, proximity handling, and tamper detection

## Runtime architecture

At startup, the controller:

1. Loads IO definitions from `ios/`
2. Loads badge files from `badges/`
3. Loads factory and user settings
4. Starts all configured readers
5. Starts all configured actions
6. Starts the life LED and launches the GUI

If `user.settings.domotics` does not exist, it is created from `factory.settings.domotics` on first start.

Reader and action workers run on dedicated threads. The GUI runs in desktop window mode when `DISPLAY` or `WAYLAND_DISPLAY` is available, otherwise it attempts GLFW null-platform mode for framebuffer-style deployments.

## Project layout

Important locations:

- `src/control.cpp`: application entrypoint and startup sequence
- `src/settings.cpp`: settings parsing and serialization
- `src/Controller/reader.cpp`: local and TCP reader handling
- `src/Controller/action.cpp`: GPIO or IP-triggered action execution
- `src/screen.cpp`: GUI bootstrapping and page wiring
- `src/GUI/Pages/`: Overview, Badges, Settings, Manual Control, Logging, Screensaver pages
- `badges/`: one JSON file per badge
- `ios/`: IO, status LED, life LED, and tamper configuration
- `factory.settings.domotics`: factory defaults
- `user.settings.domotics`: runtime/user overrides
- `libs/LIB-remote/`: local badge and GPIO TCP transport library submodule used by readers, actions, and IP IO

## Building

This repository is built with the provided `Makefile`.

### Requirements

- `make`
- A C++17 compiler available as `g++` or through `CROSS_COMPILE`
- Local submodules in `libs/` (`LIB-remote`, `LIB-funcmod`, `nanogui`)
- NanoGUI headers and libraries reachable through `HOST_DIR` and `TARGET_DIR`

The Makefile expects NanoGUI and related headers under paths such as:

- `$(HOST_DIR)/include`
- `$(HOST_DIR)/include/nanovg`
- `$(HOST_DIR)/include/nanogui/ext/nanovg/src`
- `$(HOST_DIR)/usr/lib`
- `$(TARGET_DIR)/usr/lib`

### Build commands

```bash
make
make debug
make release
make local
make clean
```

Build mode matrix:

| Command | Typical use | Dependency source |
| --- | --- | --- |
| `make` | Standard target/host build | System `HOST_DIR`/`TARGET_DIR` paths first, then `libs/` fallback if missing |
| `make debug` | Debuggable build (`-O0 -g`) | Same resolution as `make` |
| `make release` | Optimized build (`-O2`) | Same resolution as `make` |
| `make local` | Developer machine with sibling repos | `../nanogui_mod`, `../LIB-remote`, `../LIB-funcMod` first; no `libs/` fallback when those satisfy headers/libs |
| `make clean` | Remove generated artifacts | Cleans project output and local fallback builds under `libs/*/build` |

`make local` is intended for a developer setup with sibling repositories and uses:

- `../nanogui_mod`
- `../LIB-remote`
- `../LIB-funcMod`

When these local paths contain the required headers and libraries, the Makefile does not build fallback dependencies from `libs/`.

Optional manual dependency build:

```bash
make -C libs/LIB-remote static
make -C libs/LIB-funcmod static
cmake -S libs/nanogui -B libs/nanogui/build && cmake --build libs/nanogui/build
```

Cross-compilation is supported via `CROSS_COMPILE`:

```bash
make CROSS_COMPILE=arm-linux-gnueabihf-
```

The build output is written to `build-make/master-control` and symlinked as `./master-control` in the project root.

The main binary links against `libremote`, `libfuncmod`, and `libnanogui`.

If these dependencies cannot be found in configured include/library paths, the Makefile falls back to building them from `libs/`:

- `libs/LIB-remote` via its Makefile
- `libs/LIB-funcmod` via its Makefile
- `libs/nanogui` via CMake

`make clean` removes both project build artifacts and locally built fallback dependency outputs under `libs/*/build`.

Useful target:

```bash
make master-control-linter
```

## Running

Run from the project root after a successful build:

```bash
./master-control
```

The controller reads runtime files from the configured shared/etc locations and expects:

- `ios/` definitions
- `badges/` badge files
- `factory.settings.domotics` defaults
- `user.settings.domotics` overrides (auto-created on first run)

## Configuration

### Settings files

The main settings files are `factory.settings.domotics` and `user.settings.domotics`.

Structure:

```json
{
	"type": "USER",
	"version": 1,
	"network": {
		"dhcp": false,
		"dns1": "",
		"dns2": "",
		"gateway": "192.168.1.1",
		"ipAddress": "192.168.1.100",
		"netmask": "255.255.255.0"
	},
	"readers": [],
	"actions": []
}
```

### Reader configuration

Supported reader location types:

- `location_type: "LOCAL"`: local reader device
- `location_type: "IP"`: TCP listener for remote readers

Supported reader types:

- `type: "WIEGAND"`: implemented
- `type: "OSDP"`: placeholder only

For `location_type: "IP"`, `location` must be one of:

- `"<port>"` to bind `0.0.0.0:<port>`
- `"<ipv4>:<port>"` such as `"127.0.0.1:9000"`

Example reader:

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

### Action configuration

Actions poll an input and execute one or more outputs when the input becomes active.

Supported input types:

- `input_type: "GPIO"`
- `input_type: "IP"`

Example action:

```json
{
	"name": "Bel",
	"input_type": "GPIO",
	"input": "in_bel",
	"outputs": [
		{
			"output": "out_bel",
			"duration": 1000
		}
	]
}
```

Outputs may also target remote GPIO endpoints by using an IO with `type: "IP"`. In that case `Action::executeSingle()` sends `name`, `value`, and `duration` through `LIB-remote` instead of toggling a local sysfs GPIO directly.

### Badge files

Each file under `badges/` contains one badge record. Badge validation is a direct equality check against `badgeNumber`.

Example badge file:

```json
{
	"firstName": "Ayrton",
	"lastName": "Leyssens",
	"badgeNumber": 12345
}
```

### IO configuration

The `ios/` directory contains multiple JSON files.

General-purpose IO files must define an `IOs` array:

```json
{
	"IOs": [
		{
			"type": "LOCAL",
			"direction": "IN",
			"name": "in_1",
			"flank": "UP",
			"location": "/sys/class/gpio512"
		},
		{
			"type": "LOCAL",
			"direction": "OUT",
			"name": "out_bel",
			"location": "/sys/class/gpio520"
		}
	]
}
```

For IO entries with `type: "IP"` and input directions (`IN` or `AIN`), the controller starts a TCP listener during initialization. Incoming values are received through `LIB-remote`, latched if changed, and consumed on the next `get()` call.

Special files are consumed directly by peripherals:

- `ios/leds.json`: status LED and life LED mapping
- `ios/system.json`: tamper switch configuration

## TCP badge protocol

When a network reader is configured, the controller listens on the configured TCP socket through `LIB-remote`. Each request must contain a single badge payload.

Supported request formats:

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

The reply is a JSON object containing:

- `valid`: `true` when the badge exists in `badges/`
- `badge`: parsed badge number, or `0` on invalid payload
- `action`: array of output objects from the granted or denied action
- `error`: included only when the payload is invalid

Example valid badge reply:

```json
{"valid":true,"badge":123456,"action":[{"output":"out_green","duration":1000}]}
```

Example invalid badge reply:

```json
{"valid":false,"badge":123456,"action":[{"output":"out_red","duration":1000}]}
```

Example invalid payload reply:

```json
{"valid":false,"badge":0,"action":[],"error":"invalid badge payload"}
```

Quick test with `nc`:

```bash
printf '123456\n' | nc 127.0.0.1 9000
printf '{"badge":123456}\n' | nc 127.0.0.1 9000
```

## Remote GPIO protocol

For IP IO listeners and remote IP outputs, the controller uses `RemoteGpioChannel` from `LIB-remote`.

Supported command payload:

```json
{"name":"out_bel","value":1,"duration":1000}
```

Successful reply:

```json
{"ok":true}
```

Failure reply:

```json
{"ok":false,"error":"name mismatch"}
```

## GUI

The GUI is NanoGUI-based and currently includes pages for:

- landing screen
- badges
- overview
- settings
- manual control
- logging
- screensaver

The GUI also integrates:

- proximity-based screensaver wake behavior
- backlight brightness control when available
- tamper indication overlay
- runtime badge cache reload from the badge management page
