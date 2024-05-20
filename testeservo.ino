#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESP32Servo.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <SPIFFS.h>

// Configuração WiFi
const char* ssid = "VIVOFIBRA-35E2";
const char* password = "33d71f35e2";

// Configuração do NTP
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3 * 3600;
const int daylightOffset_sec = 0;

// Declaração de objetos e variáveis globais
AsyncWebServer server(80);
Servo servo1;
Servo servo2;
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer, gmtOffset_sec, daylightOffset_sec);

bool isServo1At0 = false;
bool isServo2At0 = false;
int servo1Hour = 12;
int servo1Minute = 30;
int servo2Hour = 15;
int servo2Minute = 45;

void setup() {
  // Inicialização serial e WiFi
  Serial.begin(115200);
  servo1.attach(32);
  servo2.attach(33);

  if(!SPIFFS.begin(true)){
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

  // Servir arquivos estáticos
  server.serveStatic("/", SPIFFS, "/").setDefaultFile("index.html");

  // Endpoints para atualização de horário dos servos
  server.on("/updateServo1Time", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("hour") && request->hasParam("minute")) {
      servo1Hour = request->getParam("hour")->value().toInt();
      servo1Minute = request->getParam("minute")->value().toInt();
    }
    request->send(200);
  });

  server.on("/updateServo2Time", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("hour") && request->hasParam("minute")) {
      servo2Hour = request->getParam("hour")->value().toInt();
      servo2Minute = request->getParam("minute")->value().toInt();
    }
    request->send(200);
  });

  server.begin();
}

void loop() {
  timeClient.update();
  int currentHour = timeClient.getHours();
  int currentMinute = timeClient.getMinutes();
  
  // Controle de servos baseado no horário
  if (currentHour == servo1Hour && currentMinute == servo1Minute) {
    moveServo1();
  }
  if (currentHour == servo2Hour && currentMinute == servo2Minute) {
    moveServo2();
  }

  delay(1000);
}

void moveServo1() {
  if (!isServo1At0) {
    servo1.write(180);
    isServo1At0 = true;
    delay(3000);
    servo1.write(0);
    isServo1At0 = false;
    Serial.println("Servo 1 Move");
  }
}

void moveServo2() {
  if (!isServo2At0) {
    servo2.write(180);
    isServo2At0 = true;
    delay(3000);
    servo2.write(0);
    isServo2At0 = false;
    Serial.println("Servo 2 Move");
  }
}
