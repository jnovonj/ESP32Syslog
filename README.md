# ESP32Syslog

**A robust, industrial-grade Syslog library for ESP32 (S3, C3, and classic) with full RFC 5424 support and offline buffering.**

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Version](https://img.shields.io/badge/version-1.0.1-blue.svg)](https://github.com/jnovonj/ESP32Syslog)
[![Framework: Arduino](https://img.shields.io/badge/Framework-Arduino-blue.svg)](https://www.arduino.cc/)

`ESP32Syslog` is a lightweight yet powerful logging library that allows your IoT devices to communicate with standard Syslog servers (like rsyslog, Graylog, or Logstash). Unlike simpler libraries, it implements the modern **RFC 5424** standard, allowing for structured data and precise telemetry.

---

## 🚀 Key Features

* **Dual Protocol Support**: Choose between the classic **RFC 3164** (BSD) and the modern **RFC 5424**.
* **Smart Offline Buffer**: Includes a circular buffer that stores up to 64 messages (configurable) in RAM if WiFi is lost. They are automatically flushed once the connection is restored.
* **Structured Data (RFC 5424)**: Native support for metadata blocks (e.g., `[hw@32473 ram="256000"]`) for automated indexing in professional log collectors.
* **Non-Blocking Design**: Optimized for the ESP32 network stack (`lwIP`). It manages timing using `yield()` and smart delays to prevent `ENOMEM` (Error 12) during high-frequency logging.
* **Injectable Serial Stream**: Redirect logs to any `Stream` object (Serial, SoftwareSerial, or even SD card files) via `setSerial()`.
* **Tag Auto-Extraction**: Automatically extracts tags from `[Tag] Message` format to populate the `MSGID` field in RFC 5424.

---

## 📦 Installation

### PlatformIO (Recommended)
Add the following to your `platformio.ini`:
```ini
lib_deps =
    jnovonj/ESP32Syslog
