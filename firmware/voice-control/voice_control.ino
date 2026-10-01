#include "../../config.h"
#include <WiFi.h>
#include <WebServer.h>

const char* ssid = VOICE_WIFI_SSID;
const char* password = VOICE_WIFI_PASSWORD;

WebServer server(80);

#define PLAY_PIN 4
#define REC_PIN 5

// หน้าเว็บ
void handleRoot() {
String html = "<h1>ISD1820 Control</h1>";
html += "<a href=\"/play\"><button style='font-size:30px;'>PLAY</button></a><br><br>";
html += "<a href=\"/record\"><button style='font-size:30px;'>RECORD 5s</button></a>";
server.send(200, "text/html", html);
}

// เล่นเสียง
void handlePlay() {
digitalWrite(PLAY_PIN, HIGH);
delay(200);
digitalWrite(PLAY_PIN, LOW);
server.send(200, "text/plain", "Playing");
}

// อัดเสียง 5 วินาที
void handleRecord() {
digitalWrite(REC_PIN, HIGH);
delay(5000); // อัด 5 วินาที
digitalWrite(REC_PIN, LOW);
server.send(200, "text/plain", "Recorded 5 seconds");
}

void setup() {
pinMode(PLAY_PIN, OUTPUT);
pinMode(REC_PIN, OUTPUT);

WiFi.begin(ssid, password);
while (WiFi.status() != WL_CONNECTED);

server.on("/", handleRoot);
server.on("/play", handlePlay);
server.on("/record", handleRecord);
server.begin();
}

void loop() {
server.handleClient();
}