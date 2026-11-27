# ESP32Syslog Documentation

## Getting Started
To start logging, follow these 3 simple steps:

1. **Initialize** the object and WiFi.
2. **Begin** the syslog service with your server details.
3. **Call `processBuffer()`** in your `loop()` to keep the network queue flowing.

### Basic Usage Example
```cpp
#include <ESP32Syslog.h>

ESP32Syslog syslog; // Required for macros

void setup() {
  Serial.begin(115200);
  // ... connect WiFi first ...
  
  syslog.begin(IPAddress(192, 168, 1, 100), 514, "ESP32", "MyApp");
  syslog.setSerial(&Serial);
}

void loop() {
  syslog.processBuffer(); // Crucial!
  SYSLOG_I("SYS", "Heartbeat: %d", millis());
  
  delay(5000);
}
```

---

## Build Configuration (platformio.ini)
To save program memory, you can limit the supported protocols using `build_flags`:
- `-D SYSLOG_ENABLE_RFC3164` : Enable only RFC 3164 protocol.
- `-D SYSLOG_ENABLE_RFC5424` : Enable only RFC 5424 protocol.
*If neither is defined, both are enabled by default.*

---

## Configuration API

#### `begin`
Initializes the library.
```cpp
void begin(IPAddress serverIp, uint16_t port, const char* hostname, const char* appName, SyslogProtocol protocol = _SYSLOG_DEFAULT_PROTOCOL);
```

#### `setSerial`
Defines the output stream for serial echoing.
```cpp
void setSerial(Stream* serial);
```

#### `setProcId` (**RFC 5424 only**)
Sets the Process ID.
```cpp
void setProcId(const char* procId);
```

#### `setStructuredData` (**RFC 5424 only**)
Adds RFC 5424 structured data blocks.
```cpp
void setStructuredData(const char* data);
```

---

## Protocol Comparison

| Feature | RFC 3164 (BSD) | RFC 5424 (Modern) |
| :--- | :---: | :---: |
| **Timestamp Format** | `Mmm dd hh:mm:ss` | ISO 8601 (`YYYY-MM-DDTHH:MM:SSZ`) |
| **Process ID** | Not supported | Supported |
| **Structured Data** | Not supported | Supported |

---

## Logging Methods

### printf-style methods (Class instance)
- `logX(tag, fmt, ...)` (Emergency 0)
- `logA(tag, fmt, ...)` (Alert 1)
- `logC(tag, fmt, ...)` (Critical 2)
- `logE(tag, fmt, ...)` (Error 3)
- `logW(tag, fmt, ...)` (Warning 4)
- `logN(tag, fmt, ...)` (Notice 5)
- `logI(tag, fmt, ...)` (Info 6)
- `logD(tag, fmt, ...)` (Debug 7)

### Redirection Macros (Plan B)
Use these to log to `Serial` and `Syslog` simultaneously.
- `SYSLOG_X(...)` (Emergency 0)
- `SYSLOG_A(...)` (Alert 1)
- `SYSLOG_C(...)` (Critical 2)
- `SYSLOG_E(...)` (Error 3)
- `SYSLOG_W(...)` (Warning 4)
- `SYSLOG_N(...)` (Notice 5)
- `SYSLOG_I(...)` (Info 6)
- `SYSLOG_D(...)` (Debug 7)

---

## Maintenance
#### `processBuffer`
**Crucial:** Must be called inside `loop()`. Handles buffer flushing and network connectivity checks.
```cpp
void processBuffer();
```