# PocketCore

PocketCore is a portable embedded multitool built around the ESP32-S3 and written in C++ using ESP-IDF.

The goal of the project is to build a real embedded device while practicing C++, hardware interfaces, embedded architecture, debugging, and communication protocols.

The current prototype is assembled on a breadboard.

## Current milestone

The first development milestone focuses on the basic user interface and input system.

Currently working:

- ESP32-S3 firmware build and flashing
- ST7789 display
- basic graphics
- bitmap text rendering
- six navigation buttons
- button debounce
- press, release, and long-press events
- button event queue
- button-controlled main menu
- cyclic menu navigation
- selected item highlighting
- test screen
- startup screen
- basic error screen

## Hardware

Current hardware:

- ESP32-S3 DevKitC-1 N16R8
- 16 MB Flash
- 8 MB PSRAM
- ST7789 240×320 SPI display
- 6 navigation buttons
- breadboard prototype

More peripherals will be added incrementally after the current hardware is stable.

## GPIO mapping

### Buttons

| Function | GPIO |
| --- | ---: |
| OK | 1 |
| Back | 2 |
| Up | 4 |
| Left | 5 |
| Right | 6 |
| Down | 7 |

### ST7789 display

| Function | GPIO |
| --- | ---: |
| RST | 8 |
| DC | 9 |
| CS | 10 |
| MOSI / DIN | 11 |
| SCLK / CLK | 12 |

### Reserved

| Function | GPIO |
| --- | ---: |
| BOOT | 0 |
| USB D- | 19 |
| USB D+ | 20 |
| UART0 TX | 43 |
| UART0 RX | 44 |

## Display driver

The current `Display` C++ class provides:

- ST7789 initialization
- SPI communication
- pixel drawing
- line drawing
- filled rectangles
- full-screen fill
- 5×7 bitmap font rendering
- uppercase letters
- lowercase letters
- digits
- basic symbols
- text scaling

The display currently uses RGB565 16-bit color.

## Button driver

The `ButtonDriver` currently supports six buttons:

- Up
- Down
- Left
- Right
- OK
- Back

Raw GPIO input is converted into normalized events:

- `press`
- `release`
- `long_press`

The driver also implements non-blocking debounce and stores generated events in a small circular event queue.

## Main menu

The current main menu contains:

```text
GPIO Tools
UART Terminal
I2C Scanner
Infrared
NFC
Sub-GHz
BLE Scanner
Wi-Fi Analyzer
Files
System Info
Settings
```

Up and Down move through the menu cyclically.

For example:

```text
0 -> 1 -> 2 -> ... -> 10 -> 0
```

and:

```text
0 -> 10 -> 9 -> ... -> 1 -> 0
```

Only the previously selected item and the newly selected item are redrawn during navigation instead of refreshing the entire screen.

`OK` currently opens a temporary test screen.

`Back` returns from the test screen to the main menu while preserving the selected menu item.

## Development environment

- C++
- ESP-IDF 6.0.2
- VS Code
- ESP-IDF VS Code Extension
- CMake
- Ninja
- Git

Target:

```text
esp32s3
```

## Current project structure

The project structure is being created incrementally.

New components, directories, and abstractions are added only when they have an actual use in the project.

The current code contains separate drivers for:

- display
- buttons

The application framework and individual PocketCore applications will be introduced in later milestones.
