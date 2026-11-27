# ESP32Syslog

**A robust, industrial-grade Syslog library for ESP32 (S3, C3, and classic) with full RFC 5424 support and offline buffering.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.2.1-blue.svg)](https://github.com/jnovonj/ESP32Syslog)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

`ESP32Syslog` is a lightweight yet powerful logging library that allows your IoT devices to communicate with standard Syslog servers. Unlike simpler libraries, it implements the modern **RFC 5424** standard.

## 🚀 Key Changes in v1.2.1
- **System Stability:** Refactored internal protocol dispatcher into helper methods to resolve memory access issues (`LoadProhibited`).
- **Refactoring:** Improved architectural cleanliness by separating protocol logic.
- **Documentation:** Added comprehensive API documentation in `docs/API.md` and updated usage examples.

## 🚀 Key Features
* **Dual Protocol Support**: Choose between the classic **RFC 3164** (BSD) and the modern **RFC 5424**.
* **Smart Offline Buffer**: Includes a circular buffer to store messages if WiFi is lost, with automatic flushing on reconnect.
* **Structured Data (RFC 5424)**: Native support for metadata blocks for automated indexing.
* **Non-Blocking Design**: Optimized for the ESP32 network stack (`lwIP`).
* **Injectable Serial Stream**: Redirect logs to any `Stream` object.

## 📦 Installation

### PlatformIO (Recommended)
Add the following to your `platformio.ini`:
```ini
lib_deps =
    jnovonj/ESP32Syslog
```

## 💡 Unified Usage Examples
All examples are now structured homogeneously for both protocols. Follow the basic pattern:
1. `begin()` initialization.
2. `setSerial(&Serial)` for local debugging.
3. Call `processBuffer()` in your `loop()`.










## 🚀 Key Changes in v1.2.0
- **System Stability:** Fixed critical memory access issues (`LoadProhibited`) by refactoring the internal protocol dispatcher.
- **Architecture:** The library now uses a clean, protocol-specific helper architecture, improving reliability and making it easier to add new standards.
- **Robustness:** Added defensive checks for uninitialized pointers and undefined protocol configurations.

---
