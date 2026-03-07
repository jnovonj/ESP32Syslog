#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Syslog.h>

// --- Configuration ---
// Replace with your network credentials
const char* WIFI_SSID = "YOUR_WIFI_SSID";
const char* WIFI_PASS = "YOUR_WIFI_PASSWORD";

// IP Address of your Syslog server
IPAddress syslogServer(192, 168, 1, 100);

// Syslog port (default 514)
const int SYSLOG_PORT = 514;

// Device and application names for the logs
const char* DEVICE_HOSTNAME = "ESP32Syslog";
const char* APP_NAME = "MyApp";

// Create an instance of the library.
// The object must be named 'syslog' for the macros to find it automatically.
ESP32Syslog syslog;

void setup() {
  Serial.begin(115200);
  
  // A simple pause to give the serial monitor time to connect.
  delay(1000);
  Serial.println("\n--- ESP32Syslog Basic Example ---");

  // --- WiFi Connection ---
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());

  // --- Syslog Initialization ---
  // Initialize with: Server IP, Port, Device Name, App Name, Protocol
  syslog.begin(syslogServer, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, SYSLOG_PROTO_RFC3164);
  
  // Optional: Enable log output to the serial monitor for debugging
  // We pass a pointer (&Serial) instead of true/false. 
  // This allows selecting any Stream (like Serial1, Serial2, or SD files) or NULL to disable.
  syslog.setSerial(&Serial);

  // --- Sending Logs ---
  // It's good practice to send an initial log message.
  SYSLOG_I("System", "Initialization complete. IP: %s", WiFi.localIP().toString().c_str());
}

void loop() {
  // Send an example log every 5 seconds.
  SYSLOG_I("Loop", "Uptime: %lu ms", millis());

  // It is CRUCIAL to call this function in the loop.
  // It handles the offline buffer and retries failed sends.
  syslog.processBuffer();
  
  delay(10000);
}