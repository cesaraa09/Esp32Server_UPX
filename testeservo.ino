#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPIFFS.h>
#include <AsyncTCP.h>
#include <HX711.h>
#include <Ticker.h>
#include <ArduinoJson.h>
#include <vector>

const char* ssid = "VIVOFIBRA-35E2";
const char* password = "33d71f35e2";

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
Servo servo1;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

bool isServo1At0 = false;
struct Schedule {
  String name;
  int hour;
  int minute;
  int second;
};
std::vector<Schedule> schedules;

#define pinDT 25
#define pinSCK 33

HX711 scale;
float medida = 0;
Ticker scaleTicker;

void notifyClients() {
  String message = "{\"medida\":" + String(medida, 3) + "}"; // Ajustar a precis�o aqui
  ws.textAll(message);
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo *)arg;
  if (info->opcode == WS_TEXT) {
    data[len] = 0;
    String message = (char *)data;

    StaticJsonDocument<1024> doc;
    deserializeJson(doc, message);

    if (doc["action"] == "saveSchedules") {
      schedules.clear();
      for (JsonObject schedule : doc["schedules"].as<JsonArray>()) {
        Schedule newSchedule;
        newSchedule.name = schedule["name"].as<String>();
        newSchedule.hour = schedule["hour"].as<int>();
        newSchedule.minute = schedule["minute"].as<int>();
        newSchedule.second = schedule["second"].as<int>();
        schedules.push_back(newSchedule);
      }
    } else if (message.indexOf("moveServo") >= 0) {
      int servoNumber = message.substring(message.indexOf("servo=") + 6, message.indexOf("&position=")).toInt();
      int position = message.substring(message.indexOf("position=") + 9).toInt();
      moveServoToPosition(servoNumber, position);
    }
  }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
  } else if (type == WS_EVT_DISCONNECT) {
    Serial.println("WebSocket client disconnected");
  } else if (type == WS_EVT_DATA) {
    handleWebSocketMessage(arg, data, len);
  }
}

void readScale() {
  medida = scale.get_units(5);
  Serial.println(medida, 3); 
  notifyClients();
}

void setup() {
  Serial.begin(115200);
  servo1.attach(32);

  if (!SPIFFS.begin(true)) {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  timeClient.begin();

  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");
  server.addHandler(&ws);
  ws.onEvent(onEvent);

  server.begin();

  scale.begin(pinDT, pinSCK);
  scale.set_scale(733428);
  delay(2000);
  scale.tare();
  Serial.println("Balan�a Zerada");

  scaleTicker.attach(5, readScale);
}

void loop() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  int currentSecond = timeClient.getSeconds();

  for (const auto& schedule : schedules) {
    if (schedule.hour == currentHour && schedule.minute == currentMinute && schedule.second == currentSecond) {
      moveServo1();
    }
  }
}

void moveServo1() {
  if (!isServo1At0) {
    servo1.write(25);
    isServo1At0 = true;
    delay(3000);
    servo1.write(0);
    isServo1At0 = false;
    Serial.println("Servo 1 Move");
  }
}

void moveServoToPosition(int servoNumber, int position) {
  if (servoNumber == 1) {
    servo1.write(position);
    Serial.printf("Servo 1 moveu em %d graus\n", position);
  }
}
