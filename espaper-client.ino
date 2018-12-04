
#define EPD42
//#define EPD29

extern "C" {
  #include "user_interface.h"  // Required for wifi_station_connect() to work
}

#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h>
#ifdef EPD29
  #include <EPD_WaveShare_29.h>
#endif
#ifdef EPD42
  #include <EPD_WaveShare_42.h>
#endif

#include "settings.h"
#include "configportal.h"
#include "EspaperParser.h"

#define MINI_BLACK 0
#define MINI_WHITE 1

uint16_t palette[] = {MINI_BLACK, MINI_WHITE};

#define BITS_PER_PIXEL 1
#define USE_SERIAL Serial

#ifdef EPD29
  EPD_WaveShare29 epd(CS, RST, DC, BUSY);
  #define DEVICE_ROTATION 1
#endif
#ifdef EPD42
  EPD_WaveShare42 epd(CS, RST, DC, BUSY);
  #define DEVICE_ROTATION 2
#endif


MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

EspaperParser parser(&gfx);

void showMessage(String message) {
  gfx.init();
  gfx.fillBuffer(1);
  gfx.setColor(0);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialMT_Plain_16);
  gfx.drawString(gfx.getWidth() / 2, 20, message);
  gfx.commit();
  gfx.freeBuffer();
}

boolean connectWiFi() {
  // Wake up WiFi
  wifi_fpm_do_wakeup();
  wifi_fpm_close();

  //Serial.println("Reconnecting");
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  // https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-1/
  WiFi.persistent( true );

  if (WiFi.status() == WL_CONNECTED) return true;

  
  // this 3 lines for a fix IP-address (and faster connection)
  /*IPAddress ip(192, 168, 0, 60); 
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config (ip, gateway, subnet, dns);*/
  
  Serial.print("[");
  Serial.print(WIFI_SSID.c_str());
  Serial.print("]");
  Serial.print("[");
  Serial.print(WIFI_PASS.c_str());
  Serial.print("]");

  if (WIFI_PASS == NULL || WIFI_PASS == "" || WIFI_PASS.length() == 0) {
    Serial.println("Only SSID without password");
    WiFi.begin(WIFI_SSID.c_str());
  } else {
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  }

  int i = 0;
  uint32_t startTime = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(50);
    i++;
    if (millis() - startTime > 20000) {
      Serial.println("Could not connect to WiFi");
      return false;
    }
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  //#ifndef DEV_ENV
    Serial.println("Synchronizing time for certificate check:");
    configTime(8 * 3600, 0, NTP_SERVERS);
    time_t now = time(nullptr);
    while (now < 8 * 3600 * 2) {
      delay(100);
      Serial.print(".");
      now = time(nullptr);
    }
    Serial.println("");
    struct tm timeinfo;
    gmtime_r(&now, &timeinfo);
    Serial.print("Current time: ");
    Serial.print(asctime(&timeinfo));
    Serial.println("Time Sync'ed");
  //#endif 

  return true;
}

void formatFileSystem() {
  showMessage("File System error.\nFormatting File System\nPlease wait...");
  Serial.println("Formating FS..");
  SPIFFS.format();
  Serial.println("Done Formatting");
  SPIFFS.begin();
  showMessage("Done formatting...");
}

void sleep() {
  epd.Sleep();
  Serial.printf("\n\n***Time before going to sleep %d\n", millis());
  ESP.deepSleep(UPDATE_INTERVAL_SECS * 1000000, WAKE_RF_DEFAULT );
}

void setup() {
  USE_SERIAL.begin(115200);
  // Turn of WiFi until we need it
  //WiFi.mode( WIFI_OFF );
  //WiFi.forceSleepBegin();
  //delay( 1 );

  Serial.println(ESP.getFreeHeap());
  Serial.println("Wake up!");
  
  gfx.setRotation(DEVICE_ROTATION);
  gfx.setFastRefresh(false);
  
  pinMode(USR_BTN, INPUT_PULLUP);
  int btnState = digitalRead(USR_BTN);
  Serial.println("Check FS");
  boolean isMounted = SPIFFS.begin();
  //SPIFFS.format();
  if (!isMounted) {
    formatFileSystem();
  }
  boolean isConfigLoaded = loadConfig();

  Serial.println(ESP.getFreeHeap());
  #ifndef DEV_ENV 
    parser.setRootCertificate(rootCaCert);
  #endif

  Serial.println(ESP.getFreeHeap());
  Serial.println("State: " + String(btnState));
  if (btnState == LOW || !isConfigLoaded) {
    startConfigPortal(&gfx);
  } else {
    Serial.printf("\n\n***Time before connecting to WiFi %d\n", millis());
    boolean success = connectWiFi();
    if (success) {
      Serial.printf("\n\n***Time before going to fetching data %d\n", millis());
      parser.getAndDrawScreen(SERVER_URL, REQUEST_PATH + String(DEVICE_ID) + String("/screen"), String(DEVICE_KEY), String(CLIENT_VERSION));
    } else {
      showMessage("Could not connect to WiFi...");
    }

    #ifndef DEV_ENV
        sleep();
    #endif
  }
}

void loop() {
#ifdef DEV_ENV
  boolean isPressed = !digitalRead(0);
  if (isPressed) {
    parser.getAndDrawScreen(SERVER_URL, REQUEST_PATH + String(DEVICE_ID), String(DEVICE_KEY), String(CLIENT_VERSION));
  }
  delay(100);
#endif

}
