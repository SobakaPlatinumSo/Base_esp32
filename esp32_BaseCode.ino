#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}
 
#define WIFI_SSID "TP-Link_F12C" // указывается к какой сети wifi нужно подключатся
#define WIFI_PASSWORD "12138739" //  указывается пароль для выбранной сети wifi

TimerHandle_t wifiReconnectTimer; // таймер подключения к wifi
 
AsyncWebServer server(80);

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
    break;
  case SYSTEM_EVENT_STA_DISCONNECTED: // ивент получаемый при провальном подключении wifi
    Serial.println("WiFi lost connection");
    xTimerStart(wifiReconnectTimer, 0); // запустить таймер wifi
    break;
  }
}
 
void setup(void) {
  Serial.begin(115200);
  WiFi.onEvent(WiFiEvent);
 
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });
 
  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");
}
 
void loop(void) {
  AsyncElegantOTA.loop();
}