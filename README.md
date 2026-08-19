# PocketCore

PocketCore is a portable embedded multitool built around the ESP32-S3.

## Hardware

- ESP32-S3 DevKitC-1 N16R8
- 16 MB Flash
- 8 MB PSRAM

## Development

- C++
- ESP-IDF 6.0.2
- VS Code

## Current status

- ESP-IDF project created
- ESP32-S3 target configured
- 16 MB Flash configured
- C++ application builds successfully
- Firmware flashes successfully
- Serial logging works

## GPIO mapping

| Function | GPIO | Status |
| --- | ---:| --- |
| Test LED | 4 | Used |
| USB D- | 19 | Reserved |
| USB D+ | 20 | Reserved |
| UART0 TX | 43 | Reserved |
| UART0 RX | 44 | Reserved |
| BOOT | 0 | Reserved |
