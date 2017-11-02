

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
//#include <EPD_WaveShare_42.h>
#include <EPD_WaveShare.h>
#include "EspaperParser.h"

#define CS 15  // D8
#define RST 2  // D4
#define DC 5   // D1
#define BUSY 4 // D2
#define USR_BTN 12 // D6

#define MINI_BLACK 0
#define MINI_WHITE 1

const int UPDATE_INTERVAL_SECS = 20 * 60;

uint16_t palette[] = {0, // 0
                      1 // 1
                      };

const String SERVER_URL = "http://www.squix.org/espaper/index.php";

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 296
//#define SCREEN_HEIGHT 300
//#define SCREEN_WIDTH 400
#define BITS_PER_PIXEL 1
#define USE_SERIAL Serial

EPD_WaveShare epd(EPD2_9, CS, RST, DC, BUSY);
//EPD_WaveShare42 epd(CS, RST, DC, BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

EspaperParser parser(&gfx);

ESP8266WiFiMulti WiFiMulti;



boolean connectWiFi() {
  if (WiFiMulti.run() == WL_CONNECTED) return true;

  //WiFi.begin(WIFI_SSID.c_str(),WIFI_PASS.c_str());
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP("yourssid", "yourpassw0rd");
  int i = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {
    delay(500);
    i++;
    if (i > 20) {
      Serial.println("Could not connect to WiFi");
      return false;
    }
    Serial.print(".");
  }
  return true;
}

void setup() {
  USE_SERIAL.begin(115200);
  pinMode(D3, INPUT_PULLUP);
  boolean isMounted = SPIFFS.begin();
  if (!isMounted) {
    Serial.println("Formating FS..");
    SPIFFS.format();
    Serial.println("Done Formatting");
    SPIFFS.begin();
  }
  connectWiFi();
  gfx.init();
  gfx.setRotation(1);

  parser.updateScreen(SERVER_URL + "?battery=" + String(analogRead(A0)));
  //ESP.deepSleep(UPDATE_INTERVAL_SECS * 1000000);
}

void loop() {
  boolean isPressed = !digitalRead(D3);
  if (isPressed) {
    parser.updateScreen(SERVER_URL + "?battery=" + String(analogRead(A0)));
  }
  delay(100);

}
