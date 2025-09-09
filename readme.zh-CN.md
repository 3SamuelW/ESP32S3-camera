# ESP32S3-CAM [![en](https://img.shields.io/badge/lang-en-red.svg)](README.md) [![fr](https://img.shields.io/badge/lang-fr-yellow.svg)](README.fr.md) [![zh-CN](https://img.shields.io/badge/lang-zh--CN-green.svg)](README.zh-CN.md)


[![Release](https://img.shields.io/github/v/release/3SamuelW/ESP32S3-camera?display_name=tag)](https://github.com/3SamuelW/ESP32S3-camera/releases) [![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT) [![PlatformIO](https://img.shields.io/badge/PlatformIO-ESP32--S3-orange)](https://platformio.org/) [![Last commit](https://img.shields.io/github/last-commit/3SamuelW/ESP32S3-camera)](https://github.com/3SamuelW/ESP32S3-camera/commits/main) [![GitHub stars](https://img.shields.io/github/stars/3SamuelW/ESP32S3-camera?style=social)](https://github.com/3SamuelW/ESP32S3-camera/stargazers)

---

***ESP32S3-CAM*** — 一个运行在带摄像头和显示屏的ESP32-S3开发板上的硬件+软件项目。 此仓库包含项目的PCB设计文件（Altium + Gerber/BOM）和固件（PlatformIO / Arduino）。

---

## 快速概述

- **硬件**：Altium原理图和PCB（`hardware/Altium_Designer`），BOM和Gerber（`hardware/LCEDA`）。
- **软件**：基于PlatformIO的固件在`ESP32S3-CAM/`目录下。主要项目环境是`esp32-s3-devkitc-1`。
- **构建系统**：PlatformIO（使用`espressif32`平台，`arduino`框架）。

---

## 目录

- [环境要求](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#环境要求)
- [项目结构](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#项目结构)
- [快速开始（构建和烧录）](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#快速开始构建和烧录)
- [串口监视器](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#串口监视器)
- [硬件文件和发布](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#硬件文件和发布)
- [开发说明](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#开发说明)
- [贡献](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#贡献)
- [许可证](https://claude.ai/chat/28219a8d-7a73-4137-866f-85d5bd78dcbb#许可证)

---

## 环境要求

- Git
- VS Code（推荐）+ PlatformIO IDE扩展 或安装PlatformIO Core（CLI）（`pip install -U platformio`或使用官方安装程序）
- ESP32-S3开发板（兼容`esp32-s3-devkitc-1`）
- USB线缆和相应的串口驱动

---

## 项目结构

```
ESP32S3-CAM/
├── platformio.ini 
├── convert.py
├── .gitignore
├── .pio/ 
└── src/
│ ├── main.cpp
│ ├── cameraTask.h/cpp
│ ├── webTask.h/cpp
│ ├── tfCard.h/cpp
│ ├── displayTask.h/cpp
│ ├── keyTask.h/cpp
│ ├── image.h
│ ├── config.h
└── README.md
```

---

## 快速开始（构建和烧录）

> **重要**：请检查`platformio.ini`中的`upload_port`（仓库默认值为`COM9`）。将其更改为你的实际串口（如Windows上的`COM3`或Linux上的`/dev/ttyUSB0`），或在烧录时覆盖它。

### 在VS Code中使用PlatformIO（推荐）

1. 在VS Code中打开仓库文件夹。
2. 如果尚未安装，请安装**PlatformIO IDE**扩展。
3. 在PlatformIO工具栏中，选择环境`esp32-s3-devkitc-1`。
4. 点击**Build**进行编译。
5. 点击**Upload**烧录开发板。
6. 点击**Monitor**打开串口监视器。

### 使用PlatformIO CLI

从`ESP32S3-CAM/`文件夹：

```bash
# 构建
cd ESP32S3-CAM
pio run -e esp32-s3-devkitc-1

# 烧录（将使用platformio.ini中的upload_port，除非你覆盖它）
pio run -e esp32-s3-devkitc-1 -t upload

# 如果需要，覆盖烧录端口：
pio run -e esp32-s3-devkitc-1 -t upload --upload-port COM3
```

### 故障排除

- **烧录失败**：检查`upload_port`和驱动，尝试不同的USB线缆，使用`pio run -e esp32-s3-devkitc-1 -t upload --upload-port <你的端口>`。
- **库文件缺失**：PlatformIO会下载`lib_deps`。如果网络被阻止，通过PlatformIO手动安装库或将它们放在`lib/`中。

---

## 串口监视器

```
platformio.ini`中的默认波特率：`monitor_speed = 115200
```

打开监视器：

```bash
# PlatformIO CLI：
pio device monitor -p COM9 -b 115200
# 或者
pio device monitor --port COM3 --baud 115200
```

---

## 硬件文件和发布

- **PCB设计（Altium）**：hardware/Altium_Designer/
- **BOM / Gerber / 贴片文件**：hardware/LCEDA/

> [下载硬件资料](https://github.com/3SamuelW/ESP32S3-camera/releases/download/v1.0/hardware.zip)

---

## 开发说明

- PlatformIO环境：`esp32-s3-devkitc-1`（参见`platformio.ini`）
- PSRAM已在配置中启用（`board_build.psram = true`和`-DBOARD_HAS_PSRAM`）。
- 在`lib_deps`中定义的库将在构建期间由PlatformIO自动安装（如`TFT_eSPI`，`TJpg_Decoder`）。

---

## 贡献

- Fork仓库并创建功能分支。
- 保持提交小而专注。
- 打开Pull Request描述你的更改。

> 如果你更改了开发板接线或摄像头型号，请相应地更新`README.md`和`ESP32S3-CAM/config.h`。

---

## 许可证

如果你打算开源项目，请在仓库中添加LICENSE文件（如MIT）。如果不确定，选择OSI批准的宽松许可证，如MIT。

---

## 联系方式 / 维护者

维护者：3SamuelW / [3samuelw@gmail.com](mailto:3samuelw@gmail.com)

仓库：https://github.com/3SamuelW/ESP32S3-camera