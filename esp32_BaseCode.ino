#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <Preferences.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

char WIFI_SSID[20]; // Сеть wifi к которой подключается плата
char WIFI_PASSWORD[20]; // Пароль от wifi

Preferences pref;

TimerHandle_t wifiReconnectTimer; // Таймер подключения к wifi
TimerHandle_t wifiIsNotConnect;

AsyncWebServer server(80); // Создание асинхронного сервера

String inputSSID; // Полученный с сайта ssid
String inputPassword; // Полученный с сайта пароль
String inputParam; // Поле в которое было вписанно сообщение

boolean CurrentState = 0; // Переменная в которой записано состояние платы (раздавать(0) или подключатся к сети(1))

const char *ssid = "TEST-123"; // Сеть wifi создаваемая на плате
const char *password = NULL; // Пароль от сети с платы

const char* recived_ssid = "ssid"; // Получаемая сеть wifi
const char* recived_password = "password"; // Получаемый пароль 

IPAddress apIP(192, 168, 1, 4); // ip асинхронного сервера

void ready_mode(){
  CurrentState = 1;
}

const char index_html[] PROGMEM = R"rawliteral( 
<!DOCTYPE HTML><html><head>
  <title>ESP Input Form</title>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  </head><body>
  <div>
  <button onClick="ready_mode();">Ready mode</button>
  </div>
  <form action="/get">
    recived_ssid: <input type="text" name="ssid">
    <input type="submit" value="Submit">
  </form>
  <form action="/get">
    recived_password: <input type="text" name="password">
    <input type="submit" value="Submit">
  </form><br>
</body></html>)rawliteral"; // Основная страничка сайта

void connectToWifi() // функция подключающая к wifi
{
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD); // подключится к указанной сети wifi с указанным паролем
}

void WiFiEvent(WiFiEvent_t event) // функция проверяющая подключение к wifi
{
  Serial.printf("[WiFi-event] event: %d\n", event);
  switch (event) // проверяет какой отклик получен от wifi
  {
  case SYSTEM_EVENT_STA_GOT_IP: // ивент получаемый при успешном подключении wifi
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    //CurrentState = 1;
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED: // ивент получаемый при провальном подключении wifi
    Serial.println("WiFi lost connection");
    xTimerStart(wifiReconnectTimer, 0); // запустить таймер wifi
    //CurrentState = 0;
    break;
  }
}

void setup() {
  Serial.begin(115200);

  pref.begin("ssid", false); 

  for(int i = 0; i < pref.getString("ssid_saved", "ssid is empty").length(); i++){
    WIFI_SSID[i] += pref.getString("ssid_saved", "ssid is empty")[i];
    Serial.println(WIFI_SSID);
  }

  for(int i = 0; i < pref.getString("password_saved", "password is empty").length(); i++){
    WIFI_PASSWORD[i] += pref.getString("password_saved", "password is empty")[i];
    Serial.println(WIFI_PASSWORD);
  }

//  WIFI_SSID = pref.getString("ssid_saved", "ssid is empty").c_str();

//  wifiReconnectTimer = xTimerCreate("wifiTimer", pdMS_TO_TICKS(2000), pdFALSE, (void *)0, reinterpret_cast<TimerCallbackFunction_t>(connectToWifi));

  WiFi.onEvent(WiFiEvent);

  connectToWifi();

  if (CurrentState == 1) {

    Serial.println("Current State is 1");

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hi! I am ESP32.");
    });
    Serial.println("Complete State 1");

    // AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  }

  else {

    Serial.println("Current State is 0");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    delay(100);
  
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send_P(200, "text/html", index_html);
    });

    server.on("/get", HTTP_GET, [] (AsyncWebServerRequest *request) {
      // GET input1 value on <ESP_IP>/get?input1=<inputMessage>
      if (request->hasParam(recived_ssid)) {
        inputSSID = request->getParam(recived_ssid)->value();
        inputParam = recived_ssid;
        pref.putString("ssid_saved", inputSSID);
        Serial.println(inputSSID);
      }
      if (request->hasParam(recived_password)) {
        inputPassword = request->getParam(recived_password)->value();
        inputParam = recived_password;
        pref.putString("password_saved", inputPassword);
        Serial.println(inputPassword);
      }
      request->send(200, "text/html", "HTTP GET request sent to your ESP on input field (" 
                                     + inputParam + ") <br><a href=\"/\">Return to Home Page</a>");
    });
  };
  
  Serial.println(pref.getString("ssid_saved", "ssid is empty"));
  Serial.println(pref.getString("password_saved", "password is empty"));
  Serial.println(CurrentState);
  server.begin();
}

void loop() {
  // AsyncElegantOTA.loop();
}