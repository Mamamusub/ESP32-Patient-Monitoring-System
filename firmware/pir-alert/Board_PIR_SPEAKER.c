#include "../../config.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "time.h"

// ===== WiFi =====
const char* ssid = PIR_WIFI_SSID;
const char* password = PIR_WIFI_PASSWORD;

// ===== LINE Messaging API =====
String CHANNEL_ACCESS_TOKEN = LINE_CHANNEL_ACCESS_TOKEN;
String USER_ID = LINE_USER_ID;

// ===== IFTTT Webhook =====
String iftttURL = IFTTT_WEBHOOK_URL;

// ===== PIR =====
int pirPin = 2;

// ===== Time =====
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 7 * 3600;
const int daylightOffset_sec = 0;

unsigned long lastAlert = 0;
const unsigned long alertInterval = 15000; // 30 วินาที

// ---- escape JSON ----
String escapeJson(String s) {
  s.replace("\\", "\\\\");
  s.replace("\"", "\\\"");
  s.replace("\n", "\\n");
  return s;
}

void setup() {

  Serial.begin(115200);

  pinMode(pirPin, INPUT);

  Serial.println("Connecting WiFi...");
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  Serial.println("\nWiFi Connected");

  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);

  struct tm timeinfo;
  while (!getLocalTime(&timeinfo)) {
    Serial.println("Waiting time...");
    delay(500);
  }

  Serial.println("Time synced");
}

void loop() {

  bool motion = digitalRead(pirPin);

  if (motion == HIGH && (millis() - lastAlert > alertInterval)) {

    lastAlert = millis();

    Serial.println("Motion Detected!");

    String msg =
      "แจ้งเตือน\nวันที่เวลา : " +
      getThaiTime() +
      "\nดูลิงก์ --> " + CAMERA_MONITORING_URL;

    sendLineMessage(msg);
    triggerIFTTT();
  }

  delay(200);
}

// ===== ส่ง LINE =====
void sendLineMessage(String message) {

  message = escapeJson(message);

  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  http.begin(client, "https://api.line.me/v2/bot/message/push");

  http.addHeader("Content-Type", "application/json");
  http.addHeader("Authorization", "Bearer " + CHANNEL_ACCESS_TOKEN);

  String payload =
  "{\"to\":\"" + USER_ID + "\",\"messages\":[{\"type\":\"text\",\"text\":\"" + message + "\"}]}";

  int httpCode = http.POST(payload);

  Serial.print("LINE Response: ");
  Serial.println(httpCode);

  Serial.println(http.getString());

  http.end();
}

// ===== เรียก IFTTT =====
void triggerIFTTT() {

  HTTPClient http;

  http.begin(iftttURL);

  int httpCode = http.GET();

  Serial.print("IFTTT Response: ");
  Serial.println(httpCode);

  http.end();
}

// ===== เวลาไทย =====
String getThaiTime() {

  struct tm timeinfo;

  if (!getLocalTime(&timeinfo)) return "เวลาไม่ถูกต้อง";

  const char* wday[] = {
    "วันอาทิตย์","วันจันทร์","วันอังคาร",
    "วันพุธ","วันพฤหัสบดี","วันศุกร์","วันเสาร์"
  };

  const char* monthTH[] = {
    "มกราคม","กุมภาพันธ์","มีนาคม","เมษายน",
    "พฤษภาคม","มิถุนายน","กรกฎาคม","สิงหาคม",
    "กันยายน","ตุลาคม","พฤศจิกายน","ธันวาคม"
  };

  char timeBuff[20];
  strftime(timeBuff, sizeof(timeBuff), "%H:%M:%S", &timeinfo);

  int buddhistYear = timeinfo.tm_year + 1900 + 543;

  String t =
    String(wday[timeinfo.tm_wday]) + " ที่ " +
    String(timeinfo.tm_mday) + " " +
    String(monthTH[timeinfo.tm_mon]) + " " +
    String(buddhistYear) +
    "\nเวลา " + String(timeBuff) + " น.";

  return t;
}
