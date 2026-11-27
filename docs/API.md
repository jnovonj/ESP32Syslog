# ESP32Syslog API Reference

## Class: ESP32Syslog

### Configuration

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
*Example: `syslog.setSerial(&Serial);`*

#### `setProcId`
Sets the Process ID for RFC 5424 logs.
```cpp
void setProcId(const char* procId);
```

#### `setStructuredData`
Adds RFC 5424 structured data blocks.
```cpp
void setStructuredData(const char* data);
```

---

### Logging (printf style)
These methods support standard C `printf` formatting. They require a `tag` as the first argument.

- `logI(tag, format, ...)` - Info (6)
- `logE(tag, format, ...)` - Error (3)
- `logW(tag, format, ...)` - Warning (4)
- `logD(tag, format, ...)` - Debug (7)
- `logA(tag, format, ...)` - Alert (1)
- `logC(tag, format, ...)` - Critical (2)
- `logX(tag, format, ...)` - Emergency (0)
- `logN(tag, format, ...)` - Notice (5)

---

### Maintenance

#### `processBuffer`
**Crucial:** This must be called inside the `loop()` function. It manages the circular buffer, handles WiFi reconnection logic, and transmits data to the server.
```cpp
void processBuffer();
```
