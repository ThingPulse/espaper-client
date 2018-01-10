

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <EPD_WaveShare_29.h>
#include "settings.h"
#include "configportal.h"
#include "EspaperParser.h"

#define MINI_BLACK 0
#define MINI_WHITE 1

uint16_t palette[] = {MINI_BLACK, MINI_WHITE};

#define SCREEN_HEIGHT 128
#define SCREEN_WIDTH 296
//#define SCREEN_HEIGHT 300
//#define SCREEN_WIDTH 400
#define BITS_PER_PIXEL 1
#define USE_SERIAL Serial

EPD_WaveShare29 epd(CS, RST, DC, BUSY);
//EPD_WaveShare42 epd(CS, RST, DC, BUSY);
MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

EspaperParser parser(&gfx);

ESP8266WiFiMulti WiFiMulti;

boolean connectWiFi() {
  // Wake up WiFi
  WiFi.forceSleepWake();
  delay( 1 );

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  // https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-1/
  WiFi.persistent( false );
  
  if (WiFiMulti.run() == WL_CONNECTED) return true;

  WiFi.mode(WIFI_STA);

  WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
    i++;
    if (i > 80) {
      Serial.println("Could not connect to WiFi");
      return false;
    }
    Serial.print(".");
  }
  return true;
}

void formatFileSystem() {
    gfx.fillBuffer(1);
    gfx.setColor(0);
    gfx.setTextAlignment(TEXT_ALIGN_CENTER);
    gfx.setFont(ArialMT_Plain_16);
    gfx.drawString(296 / 2, 20, "File System error.\nFormatting File System\nPlease wait...");
    gfx.commit();
    Serial.println("Formating FS..");
    SPIFFS.format();
    Serial.println("Done Formatting");
    SPIFFS.begin();
    gfx.fillBuffer(1);
    gfx.drawString(296 / 2, 20, "Done formatting...");
    gfx.commit();
}

void sleep() {
    WiFi.disconnect( true );
    delay( 1 );
    
    epd.Sleep();
    Serial.printf("\n\n***Time before going to sleep %d\n", millis());
    ESP.deepSleep(UPDATE_INTERVAL_SECS * 1000000, WAKE_RF_DISABLED );
}

void setup() {
  // Turn of WiFi until we need it
  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay( 1 );
  
  USE_SERIAL.begin(115200);
  gfx.init();
  gfx.setRotation(1);
  pinMode(USR_BTN, INPUT_PULLUP);
  int btnState = digitalRead(USR_BTN);

  boolean isMounted = SPIFFS.begin();
  //SPIFFS.format();
  if (!isMounted) {
    formatFileSystem();
  }
  boolean isConfigLoaded = loadConfig();


  Serial.println("State: " + String(btnState));
  if (btnState == LOW || !isConfigLoaded) {
    startConfigPortal(&gfx);
  } else {
    Serial.printf("\n\n***Time before connecting to WiFi %d\n", millis());
    connectWiFi();
    Serial.printf("\n\n***Time before going to fetching data %d\n", millis());
    parser.updateScreen(SERVER_URL, SHA1_FINGERPRINT, REQUEST_PATH + String(DEVICE_ID), String(DEVICE_KEY), String(CLIENT_VERSION));

    #ifndef DEV_ENV
      sleep();
    #endif
  }
}

void loop() {
  #ifdef DEV_ENV
    boolean isPressed = !digitalRead(0);
    if (isPressed) {
      parser.updateScreen(SERVER_URL, SHA1_FINGERPRINT, REQUEST_PATH + String(DEVICE_ID), DEVICE_KEY, CLIENT_VERSION);
    }
    delay(100);
  #endif

}
