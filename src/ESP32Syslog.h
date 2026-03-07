#ifndef ESP32_SYSLOG_H
#define ESP32_SYSLOG_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <stdarg.h>
#include <time.h>
#include <Stream.h>

// Limit configuration
#define SYSLOG_BUFFER_SIZE 64      // Circular buffer size (number of messages)
#define SYSLOG_MSG_MAX_LEN 150     // Maximum length of a Syslog message
#define SYSLOG_FACILITY 16         // local use 0 (local0)

/**
 * Standard Syslog Severities (RFC 3164)
 */
enum SyslogSeverity {
    SYSLOG_EMERG = 0,
    SYSLOG_ALERT = 1,
    SYSLOG_CRIT = 2,
    SYSLOG_ERR = 3,
    SYSLOG_WARN = 4,
    SYSLOG_NOTICE = 5,
    SYSLOG_INFO = 6,
    SYSLOG_DEBUG = 7
};

/**
 * Syslog Protocol Versions
 */
enum SyslogProtocol {
    SYSLOG_PROTO_RFC3164, // Legacy BSD format
    SYSLOG_PROTO_RFC5424  // Modern IETF format
};

/**
 * Structure for internal circular buffer (Offline Mode)
 */
struct SyslogItem {
    uint8_t severity;
    char message[SYSLOG_MSG_MAX_LEN];
};

class ESP32Syslog {
public:
    ESP32Syslog();

    // Initializes the library
    void begin(IPAddress serverIp, uint16_t port, const char* hostname, const char* appName, SyslogProtocol protocol = SYSLOG_PROTO_RFC3164);

    // Manual message sending
    void send(SyslogSeverity severity, const char* message);

    // Formatted sending methods (printf style)
    void log(SyslogSeverity severity, const char* tag, const char* format, ...);
    void vlog(SyslogSeverity severity, const char* tag, const char* format, va_list args);
    // Log methods with printf format and built-in Tag
    void logX(const char* tag, const char* format, ...);       // Emergency (0)
    void logA(const char* tag, const char* format, ...);       // Alert     (1)
    void logC(const char* tag, const char* format, ...);       // Critical  (2)
    void logE(const char* tag, const char* format, ...);       // Error     (3)
    void logW(const char* tag, const char* format, ...);       // Warning   (4)
    void logN(const char* tag, const char* format, ...);       // Notice    (5)
    void logI(const char* tag, const char* format, ...);       // Info      (6)
    void logD(const char* tag, const char* format, ...);       // Debug     (7)

    // Buffer management
    void processBuffer();

    void setSerial(Stream* serial);
    void setProcId(const char* procId);
    void setStructuredData(const char* data);

private:
    WiFiUDP _udp;
    IPAddress _serverIp;
    uint16_t _port;
    char _hostname[32];
    char _appName[32];
    char _procId[32];
    char _structuredData[128];
    SyslogProtocol _protocol;
    bool _isInitialized = false; // Control variable
    Stream* _serial_port = nullptr; // Port for Serial logging

    // Circular Buffer Management
    SyslogItem _buffer[SYSLOG_BUFFER_SIZE];
    size_t _head = 0;
    size_t _tail = 0;
    size_t _count = 0;

    void pushToBuffer(uint8_t sev, const char* msg);
    bool transmitUDP(uint8_t severity, const char* msg);
};

// --- REDIRECTION MACROS (PLAN B) ---
// These macros send the log to the Serial Monitor (via Arduino)
// and simultaneously to the Linux server via Syslog.
// extern is used so the macro finds the 'syslog' object in your .ino

extern ESP32Syslog syslog;

#define SYSLOG_X(tag, fmt, ...) syslog.logX(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_A(tag, fmt, ...) syslog.logA(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_C(tag, fmt, ...) syslog.logC(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_E(tag, fmt, ...) syslog.logE(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_W(tag, fmt, ...) syslog.logW(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_N(tag, fmt, ...) syslog.logN(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_I(tag, fmt, ...) syslog.logI(tag, fmt, ##__VA_ARGS__)
#define SYSLOG_D(tag, fmt, ...) syslog.logD(tag, fmt, ##__VA_ARGS__)

#endif