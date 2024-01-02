#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <EEPROM.h>

const char* ssid = "YOUR_SSID";
const char* password = "YOUR_PASSWORD";
IPAddress staticIP(192, 168, 1, 185);  // The static IP address to use
IPAddress gateway(192, 168, 1, 1);     // The gateway's IP address
IPAddress subnet(255, 255, 255, 0);    // The subnet mask

int relayPin = D1; // The pin connected to the relay
int onTime = 10;    // The time the relay should be on in seconds
int offTime = 10;   // The time the relay should be off in seconds

ESP8266WebServer server(8088); // The web server on port 8088

void handleRoot() {
  String html = "<style>body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; font-size: 48px;}</style>";
  html += "<html><body><h1>Pellet Feeder Timer by Mario Ancic@2023</h1>";
  html += "<form method='get' action='setTimer'>";
  html += "<label>On time (in seconds):</label>";
  html += "<input type='text' name='onTime' value='" + String(onTime) + "' size='6' style='font-size: 40px;'><br>";
  html += "<label>Off time (in minutes):</label>";
  html += "<input type='text' name='offTime' value='" + String(offTime / 60) + "' size='6' style='font-size: 40px;'><br>";
  html += "<input type='submit' value='Set Timer' style='font-size: 100px;'><br>";
  html += "</form><br><br><br><br>";
  html += "<form method='get' action='toggleRelay'>";
  html += "<input type='submit' value='Toggle Feeder Relay' style='font-size: 80px;'><br>";
  html += "</form></body></html>";
  server.send(200, "text/html", html);
}

void handleSetTimer() {
  onTime = server.arg("onTime").toInt();
  int offMinutes = server.arg("offTime").toInt();
  offTime = offMinutes * 60; // Convert minutes to seconds

  EEPROM.begin(sizeof(int) * 2);
  EEPROM.put(0, onTime);
  EEPROM.put(sizeof(int), offTime);
  EEPROM.end();

  server.send(200, "text/plain", "Timer set");
  
  
}

void handleToggleRelay() {
  digitalWrite(relayPin, !digitalRead(relayPin));
  server.send(200, "text/plain", "Relay toggled");
  
}

void setup() {
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  WiFi.config(staticIP, gateway, subnet);

  EEPROM.begin(sizeof(int) * 2);
  EEPROM.get(0, onTime);
  EEPROM.get(sizeof(int), offTime);
  EEPROM.end();

  server.on("/", handleRoot);
  server.on("/setTimer", handleSetTimer);
  server.on("/toggleRelay", handleToggleRelay);
  server.begin();
  Serial.println("Web server started");
}

void loop() {
  static unsigned long lastRelayToggle = 0;
  static bool relayState = false;
  if (millis() - lastRelayToggle >= (relayState ? onTime : offTime) * 1000) {
    lastRelayToggle = millis();
    relayState = !relayState;
    digitalWrite(relayPin, relayState);
  }
  server.handleClient();
}

