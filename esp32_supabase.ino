#include <WiFi.h>
#include <HTTPClient.h>

// --- WiFi Settings ---
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// --- Supabase Settings ---
const char* supabase_url = "https://omfldpqksqlvoeewasbv.supabase.co/rest/v1/health_readings";
const char* supabase_anon_key = "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9tZmxkcHFrc3Fsdm9lZXdhc2J2Iiwicm9sZSI6ImFub24iLCJpYXQiOjE3NzkyMTQ1MjQsImV4cCI6MjA5NDc5MDUyNH0.9bPqQkxDZ0a0AoJirDJoEL-LNroKbJAEIPQzqjYDBCY";

// --- Sensor Pins ---
#define ECG_PIN 34
#define PULSE_PIN 35

unsigned long lastUploadTime = 0;
const int uploadInterval = 1500; // Send readings every 1.5 seconds

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  WiFi.mode(WIFI_STA);
  connectToWiFi();
}

void loop() {
  // Reconnect if WiFi drops
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi disconnected. Retrying...");
    connectToWiFi();
  }

  if (millis() - lastUploadTime >= uploadInterval) {
    lastUploadTime = millis();
    
    // Read sensors
    int ecgValue = analogRead(ECG_PIN);
    int pulseValue = analogRead(PULSE_PIN);
    
    // Basic processing (replace mock logic with your actual sensor libraries)
    float temperature = 36.5 + (random(-5, 6) / 10.0);
    int bpm = 70 + random(0, 15);
    String pulseStatus = (pulseValue > 2000) ? "Detected" : "Searching";
    
    uploadToSupabase(temperature, bpm, pulseStatus, ecgValue);
  }
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected!");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void uploadToSupabase(float temp, int bpm, String pulse, int ecg) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(supabase_url);
    
    // Add required Supabase headers
    http.addHeader("Content-Type", "application/json");
    http.addHeader("apikey", supabase_anon_key);
    http.addHeader("Authorization", String("Bearer ") + supabase_anon_key);
    http.addHeader("Prefer", "return=minimal");

    // Construct lightweight JSON payload manually
    String jsonPayload = "{";
    jsonPayload += "\"temperature\":" + String(temp, 1) + ",";
    jsonPayload += "\"bpm\":" + String(bpm) + ",";
    jsonPayload += "\"pulse_status\":\"" + pulse + "\",";
    jsonPayload += "\"ecg\":" + String(ecg);
    jsonPayload += "}";

    int httpResponseCode = http.POST(jsonPayload);
    
    if (httpResponseCode > 0) {
      Serial.print("Cloud Sync Success | HTTP Code: ");
      Serial.println(httpResponseCode);
    } else {
      Serial.print("Cloud Sync Failed | Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  }
}
