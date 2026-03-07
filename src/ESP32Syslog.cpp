#include "ESP32Syslog.h"
#include <WiFi.h>

/**
 * Constructor: Initializes pointers and states
 */
ESP32Syslog::ESP32Syslog() : _protocol(SYSLOG_PROTO_RFC3164), _isInitialized(false), _serial_port(nullptr), _head(0), _tail(0), _count(0) {}

/**
 * Initial configuration
 */
void ESP32Syslog::begin(IPAddress serverIp, uint16_t port, const char* hostname, const char* appName, SyslogProtocol protocol) {
    if (_isInitialized) return;

    _serverIp = serverIp;
    _port = port;
    _protocol = protocol;

    strncpy(_hostname, hostname, sizeof(_hostname) - 1);
    _hostname[sizeof(_hostname) - 1] = '\0';
    
    strncpy(_appName, appName, sizeof(_appName) - 1);
    _appName[sizeof(_appName) - 1] = '\0';

    // Default ProcID to NILVALUE
    strcpy(_procId, "-");

    // Default Structured Data to NILVALUE
    strcpy(_structuredData, "-");

    _udp.begin(0);
    _isInitialized = true;
}

/**
 * Enable or disable Serial echo
 */
void ESP32Syslog::setSerial(Stream* serial) {
    _serial_port = serial;
}

/**
 * Set the Process ID (RFC 5424)
 */
void ESP32Syslog::setProcId(const char* procId) {
    strncpy(_procId, procId, sizeof(_procId) - 1);
    _procId[sizeof(_procId) - 1] = '\0';
}

/**
 * Set the Structured Data (RFC 5424)
 */
void ESP32Syslog::setStructuredData(const char* data) {
    if (data == nullptr || strlen(data) == 0) {
        strcpy(_structuredData, "-");
    } else {
        strncpy(_structuredData, data, sizeof(_structuredData) - 1);
        _structuredData[sizeof(_structuredData) - 1] = '\0';
    }
}

/**
 * Central formatting engine
 */
void ESP32Syslog::vlog(SyslogSeverity severity, const char* tag, const char* format, va_list args) {
    if (!_isInitialized) return;
    
    char final_msg[SYSLOG_MSG_MAX_LEN];
    char* user_msg_ptr;
    size_t remaining_space;

    // 1. Prepare the prefix based on the protocol
    // RFC 3164 needs a ":" after the tag to prevent parsing errors with later ":" in the message.
    // RFC 5424 works better with brackets as the tag is extracted to MSGID anyway.
    int prefix_len;
    if (_protocol == SYSLOG_PROTO_RFC3164) {
        prefix_len = snprintf(final_msg, sizeof(final_msg), "%s: ", tag);
    } else {
        prefix_len = snprintf(final_msg, sizeof(final_msg), "[%s] ", tag);
    }

    // Handle potential snprintf errors or truncation
    if (prefix_len < 0) {
        prefix_len = 0;
    } else if (prefix_len >= sizeof(final_msg)) {
        prefix_len = sizeof(final_msg) - 1;
    }

    // 2. Format the user's message right after the prefix
    user_msg_ptr = final_msg + prefix_len;
    remaining_space = sizeof(final_msg) - prefix_len;
    vsnprintf(user_msg_ptr, remaining_space, format, args);

    // 3. SIMULTANEOUS SEND TO SERIAL
    if (_serial_port) {
        if (_protocol == SYSLOG_PROTO_RFC5424 && strcmp(_structuredData, "-") != 0) {
            _serial_port->printf("[%s] %s %s\n", tag, _structuredData, user_msg_ptr);
        } else {
            // Keep brackets for serial output as it's more readable for humans
            _serial_port->printf("[%s] %s\n", tag, user_msg_ptr);
        }
    }

    // 4. SEND TO NETWORK QUEUE
    send(severity, final_msg);
}

/**
 * Add message to buffer and try to process
 */
void ESP32Syslog::send(SyslogSeverity severity, const char* message) {
    pushToBuffer((uint8_t)severity, message);
    processBuffer();
}

/**
 * UDP Transmission (RFC 3164)
 */
bool ESP32Syslog::transmitUDP(uint8_t severity, const char* msg) {
    int pri = (SYSLOG_FACILITY << 3) | (uint8_t)severity;

    if (_udp.beginPacket(_serverIp, _port)) {
        if (_protocol == SYSLOG_PROTO_RFC5424) {
            // RFC 5424 Format: <PRI>VERSION TIMESTAMP HOSTNAME APPNAME PROCID MSGID STRUCTURED-DATA MSG
            // Note: For this to work, time must be synchronized via NTP.
            time_t now;
            time(&now);
            char time_buf[30];
            // Format timestamp in ISO 8601 format with UTC timezone
            strftime(time_buf, sizeof(time_buf), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));

            // Extract TAG from "[Tag] Message" to use as MSGID
            char msgId[32];
            strcpy(msgId, "-");
            const char* msgBody = msg;

            if (msg[0] == '[') {
                const char* endBracket = strchr(msg, ']');
                if (endBracket != nullptr && (endBracket - msg) < (int)sizeof(msgId)) {
                    size_t len = endBracket - msg - 1;
                    if (len > 0) {
                        strncpy(msgId, msg + 1, len);
                        msgId[len] = '\0';
                    }
                    msgBody = (*(endBracket + 1) == ' ') ? endBracket + 2 : endBracket + 1;
                }
            }

            // Send RFC 5424 with ProcID, MSGID and STRUCTURED-DATA
            _udp.printf("<%d>1 %s %s %s %s %s %s %s", pri, time_buf, _hostname, _appName, _procId, msgId, _structuredData, msgBody);
        } else if (_protocol == SYSLOG_PROTO_RFC3164) {
            // RFC 3164 Format: <PRI>TIMESTAMP HOSTNAME MSG
            
            time_t now;
            time(&now);
            char time_buf[17];
            // Format timestamp "Mmm dd hh:mm:ss"
            strftime(time_buf, sizeof(time_buf), "%b %d %H:%M:%S", localtime(&now));

            _udp.printf("<%d>%s %s %s", pri, time_buf, _hostname, msg);
        }

        bool success = _udp.endPacket();
        return success;
    }
    return false;
}

/**
 * Circular Buffer Management
 */
void ESP32Syslog::pushToBuffer(uint8_t sev, const char* msg) {
    _buffer[_head].severity = sev;
    strncpy(_buffer[_head].message, msg, SYSLOG_MSG_MAX_LEN - 1);
    _buffer[_head].message[SYSLOG_MSG_MAX_LEN - 1] = '\0';
    
    _head = (_head + 1) % SYSLOG_BUFFER_SIZE;
    
    if (_count < SYSLOG_BUFFER_SIZE) {
        _count++;
    } else {
        _tail = (_tail + 1) % SYSLOG_BUFFER_SIZE; // Overwrite the oldest
    }
}

/**
 * Smart buffer flushing (WiFi Control)
 */
void ESP32Syslog::processBuffer() {
    if (!_isInitialized) return;

    // Only enter if WiFi is available
    if (WiFi.status() != WL_CONNECTED) return;

    // Flush circular buffer
    while (_count > 0 && WiFi.status() == WL_CONNECTED) {        
        if (transmitUDP(_buffer[_tail].severity, _buffer[_tail].message)) {
            _tail = (_tail + 1) % SYSLOG_BUFFER_SIZE;
            _count--;
            
            // Yield CPU time to background tasks (like the network stack).
            yield(); 
            // This short delay is crucial. It gives the lwIP network stack time to process
            // and send the previous UDP packet, preventing buffer overflows and packet loss
            // when sending logs in quick succession.
            delay(2); 
        } else {
            // If transmitUDP returns false (e.g., network buffer full),
            // stop and try again in the next loop iteration.
            break; 
        }
    }
}

// --- PUBLIC LOG METHODS ---

/**/
void ESP32Syslog::log(SyslogSeverity severity, const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(severity, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_EMERGENCY level message with format */
void ESP32Syslog::logX(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_EMERG, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_ALERT level message with format */
void ESP32Syslog::logA(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_ALERT, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_CRIT level message with format */
void ESP32Syslog::logC(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_CRIT, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_ERROR level message with format */
void ESP32Syslog::logE(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_ERR, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_WARNING level message with format */
void ESP32Syslog::logW(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_WARN, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_NOTICE level message with format */
void ESP32Syslog::logN(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_NOTICE, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_INFO level message with format */
void ESP32Syslog::logI(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_INFO, tag, format, args);
    va_end(args);
}

/* Sends a SYSLOG_DEBUG level message with format */
void ESP32Syslog::logD(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    vlog(SYSLOG_DEBUG, tag, format, args);
    va_end(args);
}