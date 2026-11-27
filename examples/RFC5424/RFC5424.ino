#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Syslog.h>
#include <time.h>

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

// NTP Server configuration for accurate timestamps (Required for RFC 5424)
const char* NTP_SERVER = "es.pool.ntp.org";
const long  GMT_OFFSET_SEC = 3600;       // GMT +1 (Adjust for your timezone)
const int   DAYLIGHT_OFFSET_SEC = 3600;  // Daylight saving time (+1h)

// Create an instance of the library.
ESP32Syslog syslog;

void setup() {
  Serial.begin(115200);
  
  // A simple pause to give the serial monitor time to connect.
  delay(1000);
  Serial.println("\n--- ESP32Syslog RFC 5424 Example ---");

  // 1. Connect to WiFi
  Serial.printf("Connecting to %s ", WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected.");
  Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());

  // 2. Configure Time via NTP
  // RFC 5424 requires accurate timestamps (ISO 8601).
  configTime(GMT_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.print("Synchronizing time...");
  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.print(".");
    delay(100);
  }
  Serial.println("\nTime synchronized!");

  // --- Syslog Initialization ---
  // Note: SYSLOG_PROTO_RFC3164 is defined only if SYSLOG_ENABLE_RFC3164 is enabled (default).
  // Initialize with: Server IP, Port, Device Name, App Name, Protocol

  syslog.begin(syslogServer, SYSLOG_PORT, DEVICE_HOSTNAME, APP_NAME, SYSLOG_PROTO_RFC5424);

  // Optional: Enable log output to the serial monitor for debugging
  // We pass a pointer (&Serial) instead of true/false. 
  // This allows selecting any Stream (like Serial1, Serial2, or SD files) or NULL to disable.
  syslog.setSerial(&Serial);
  
  // 4. Set Process ID (Optional)
  syslog.setProcId("101");

  // 5. Set Structured Data (Optional)
  syslog.setStructuredData("[exampleSDID@32473 iut=\"3\" eventSource=\"Application\" eventID=\"1011\"]");

  // --- Sending Logs ---
  SYSLOG_I("SYS", "Initialization complete.");
}

void loop() {
  uint32_t freeRam = ESP.getFreeHeap();
  uint32_t uptime = millis();
  char sdBuffer[150];
  snprintf(sdBuffer, sizeof(sdBuffer), "[hw@32473 ram=\"%u\" uptime=\"%lu\"]", freeRam, uptime);
  syslog.setStructuredData(sdBuffer);
  SYSLOG_I("LOOP", "Uptime: %lu ms", millis());

  // MUST be called in loop()
  syslog.processBuffer();
  
  delay(10000);
}