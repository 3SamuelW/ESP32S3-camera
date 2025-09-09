# ESP32S3-CAM

ESP32S3-CAM — a hardware + software project that runs on an ESP32-S3 development board with a camera and display.
This repository contains the PCB design files (Altium + Gerber/BOM) and the firmware (PlatformIO / Arduino) for the project.

---

## Quick summary

- **Hardware**: Altium schematic and PCB (`hardware/Altium_Designer`), BOM and Gerber (`hardware/LCEDA`).
- **Software**: PlatformIO-based firmware under `software/`. Main project environment is `esp32-s3-devkitc-1`.
- **Build system**: PlatformIO (uses `espressif32` platform, `arduino` framework).
- **Note**: Build artifacts (e.g. `.pio/`) and large binaries (Gerber zip, yields) should **not** be stored in Git history. Put large deliverables in GitHub Releases.

---

## Table of contents

- [Requirements](#requirements)  
- [Project structure](#project-structure)  
- [Quick start (build & upload)](#quick-start-build--upload)  
- [Serial monitor](#serial-monitor)  
- [Hardware files & Releases](#hardware-files--releases)  
- [Development notes](#development-notes)  
- [Contributing](#contributing)  
- [License](#license)

---

## Requirements

- Git
- VS Code (recommended) + PlatformIO IDE extension  
  or PlatformIO Core (CLI) installed (`pip install -U platformio` or use the official installers)
- ESP32-S3 development board (compatible with `esp32-s3-devkitc-1`)
- USB cable and appropriate serial drivers
- (Optional) SSH key or GitHub credentials for pushing

---

## Project structure

	ESP32S3-CAM/
	├─ platformio.ini 
	├─ convert.py
	├─ .gitignore
	├─ .pio/ 
	└─ src/
	│ ├─ main.cpp
	│ ├─ cameraTask.h/cpp
	│ ├─ webTask.h/cpp
	│ ├─ tfCard.h/cpp
	│ ├─ displayTask.h/cpp
	│ ├─ keyTask.h/cpp
	│ ├─ image.h
	│ ├─ config.h
	└─ README.md

---

## Quick start (build & upload)

> **Important**: check `platformio.ini` for `upload_port` (default in repo is `COM9`). Change it to your actual serial port (e.g. `COM3` on Windows or `/dev/ttyUSB0` on Linux), or override it at upload time.

### Using PlatformIO in VS Code (recommended)
1. Open the repository folder in VS Code.
2. Install the **PlatformIO IDE** extension if not already installed.
3. In the PlatformIO toolbar, choose the environment `esp32-s3-devkitc-1`.
4. Click **Build** to compile.  
5. Click **Upload** to flash the board.  
6. Click **Monitor** to open the serial monitor.

### Using PlatformIO CLI
From the `software/` folder:

```bash
# build
cd software
pio run -e esp32-s3-devkitc-1

# upload (will use upload_port from platformio.ini unless you override)
pio run -e esp32-s3-devkitc-1 -t upload

# override upload port if needed:
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

### Troubleshooting
- **Upload fails**: check `upload_port` and drivers, try a different USB cable, use `pio run -e esp32-s3-devkitc-1 -t upload --upload-port <your-port>`.
- **Missing libraries**: PlatformIO will download `lib_deps`. If network blocked, install libraries manually via PlatformIO or put them in `lib/`.
## Serial monitor
Default baud rate in `platformio.ini`: `monitor_speed = 115200`

Open monitor:
```
# PlatformIO CLI:
pio device monitor -p COM9 -b 115200
# or
pio device monitor --port COM3 --baud 115200
```

## Hardware files & Releases
- **PCB design (Altium)**: hardware/Altium_Designer/

- **BOM / Gerber / Pick-and-place**: hardware/LCEDA/

## Development notes
- PlatformIO environment: `esp32-s3-devkitc-1` (see `platformio.ini`)

- PSRAM is enabled in the config (`board_build.psram = true` and `-DBOARD_HAS_PSRAM`).

- Libraries defined in `lib_deps` will be automatically installed by PlatformIO during build (e.g. `TFT_eSPI`, `TJpg_Decoder`).

## Contributing
- Fork the repo and create a feature branch.

- Keep commits small and focused.

- Open a pull request describing your changes.

> If you change board wiring or camera model, update `README.md` and `software/config.h` accordingly.


## License
Add a LICENSE file to the repo (e.g. MIT) if you intend to open-source the project. If unsure, choose an OSI-approved permissive license such as MIT.

Contact / Maintainer
Maintainer: 3SamuelW / 3samuelw@gmail.com

Repository: https://github.com/3SamuelW/ESP32S3-camera
