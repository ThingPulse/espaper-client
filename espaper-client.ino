/**The MIT License (MIT)

 Copyright (c) 2018 by ThingPulse Ltd., https://thingpulse.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

extern "C" {
  #include "user_interface.h"  // Required for wifi_station_connect() to work
}

#include <Arduino.h>
#include <time.h>
#include <ESP8266WiFi.h>
#include "settings.h"
#ifdef EPD29
  #include <EPD_WaveShare_29.h>
#endif
#ifdef EPD42
  #include <EPD_WaveShare_42.h>
#endif
#include "configportal.h"
#include "EspaperParser.h"

#define MINI_BLACK 0
#define MINI_WHITE 1

uint16_t palette[] = {MINI_BLACK, MINI_WHITE};

#define BITS_PER_PIXEL 1

#ifdef EPD29
  EPD_WaveShare29 epd(CS, RST, DC, BUSY);
  #define DEVICE_ROTATION 1
#endif
#ifdef EPD42
  EPD_WaveShare42 epd(CS, RST, DC, BUSY);
  #define DEVICE_ROTATION 2
#endif


MiniGrafx gfx = MiniGrafx(&epd, BITS_PER_PIXEL, palette);

void showMessage(String message) {
  gfx.init();
  gfx.fillBuffer(1);
  gfx.setColor(MINI_BLACK);
  gfx.setTextAlignment(TEXT_ALIGN_CENTER);
  gfx.setFont(ArialMT_Plain_16);
  gfx.drawStringMaxWidth(gfx.getWidth() / 2, STD_MESSAGE_Y_POSITION, gfx.getWidth() * MAX_TEXT_WIDTH_FACTOR, message);
  gfx.commit();
  gfx.freeBuffer();
}

boolean connectWifi() {
  // Wake up WiFi
  wifi_fpm_do_wakeup();
  wifi_fpm_close();

  //Serial.println("Reconnecting");
  wifi_set_opmode(STATION_MODE);
  wifi_station_connect();

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  // https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-1/
  WiFi.persistent(false);

  if (WiFi.status() == WL_CONNECTED) return true;

  // this 3 lines for a fix IP-address (and faster connection)
  /*IPAddress ip(192, 168, 0, 60); 
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config (ip, gateway, subnet, dns);*/
  
  if (WIFI_PASS == "") {
    Serial.println("Only SSID without password");
    WiFi.begin(WIFI_SSID.c_str());
  } else {
    WiFi.begin(WIFI_SSID.c_str(), WIFI_PASS.c_str());
  }

  int i = 0;
  uint32_t startTime = millis();
  Serial.print("WiFi connect");
  while (WiFi.status() != WL_CONNECTED) {
    delay(300);
    i++;
    if (millis() - startTime > 20000) {
      Serial.println("\nFailed to connect to WiFi");
      return false;
    }
    Serial.print(".");
  }
  Serial.println(WiFi.localIP());
  return true;
}

bool initTime() {
  //#ifndef DEV_ENV
  Serial.print("NTP sync");
  String timezoneCode = getTimeZoneSettings();
  configTime(0, 0, getNtpServer(0).c_str(), getNtpServer(1).c_str(), getNtpServer(2).c_str());
  setenv("TZ", timezoneCode.c_str(), 0);
  tzset();

  // wait until NTP time was correctly syncronized
  uint8_t retryCounter = 0;
  time_t now;
  uint32_t startTime = millis();
  uint16_t ntpTimeoutMillis = NTP_SYNC_TIMEOUT_SECONDS * 1000;
  while((now = time(nullptr)) < NTP_MIN_VALID_EPOCH) {
    uint32_t runtimeMillis = millis() - startTime;
    if (runtimeMillis > ntpTimeoutMillis) {
      Serial.printf("\nFailed to sync time through NTP. Giving up after %dms.\n", runtimeMillis);
      return false;
    }
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.printf("Current time: %d\n", now);
  //#endif

  return true;
}

String buildRegistrationRequestBody() {
  return "{\"macAddress\": \"" + WiFi.macAddress() + "\", \"deviceType\": \"" + SERVER_API_DEVICE_TYPE + "\", \"timeZone\": \"" + getTimeZoneName() + "\"}";  
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
  ESP.deepSleep(UPDATE_INTERVAL_MINS * 60 * 1000000, WAKE_RF_DEFAULT );
}

void sleepWifi() {
  Serial.println("Putting WiFi to sleep");
  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();
  delay(1);
}

void fetchAndDrawScreen(EspaperParser *parser) {
  int httpCode = parser->getAndDrawScreen(SERVER_API_DEVICES_PATH + "/" + DEVICE_ID + "/screen", sleepWifi);
  if (httpCode == 410) {
    DEVICE_ID = "";
    DEVICE_SECRET = "";
    saveConfig();
    ESP.restart(); 
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println();
  Serial.println("Boot sequence arrived in setup()");
  // Turn WiFi off until we really need it
  sleepWifi();

  Serial.println("Current free heap: " + ESP.getFreeHeap());
  
  gfx.setRotation(DEVICE_ROTATION);
  gfx.setFastRefresh(false);
  
  pinMode(USR_BTN, INPUT_PULLUP);
  int btnState = digitalRead(USR_BTN);
  Serial.println("Checking FS");
  boolean isMounted = SPIFFS.begin();
  if (!isMounted) {
    formatFileSystem();
  }
  boolean isConfigLoaded = loadConfig();

  Serial.println("Button state: " + String(btnState));
  if (btnState == LOW || !isConfigLoaded) {
    startConfigPortal(&gfx);
  } else {
    Serial.printf("\n\n***Time before connecting to WiFi %d\n", millis());
    boolean success = connectWifi();
    if (success) {
      boolean timeInit = initTime();
      if (timeInit) {
        EspaperParser parser(&gfx, SERVER_URL, DEVICE_SECRET, String(CLIENT_VERSION));
        #ifndef DEV_ENV 
        parser.setRootCertificate(rootCaCert);
        #endif
        Serial.printf("\n\n***Time before going to fetching data %d\n", millis());
        if (isDeviceRegistered()) {
          fetchAndDrawScreen(&parser);
        } else {
          Serial.printf("Free mem: %d\n",  ESP.getFreeHeap());
          Serial.println("Device id and/or secret are not set yet -> registering device with server now");
          EspaperParser::DeviceIdAndSecret d = parser.registerDevice(SERVER_API_DEVICES_PATH, buildRegistrationRequestBody());
          if (d.deviceId == "-1") {
            Serial.println("Device registration failed");
            showMessage("Sorry, device registration failed. Please ensure the device has access to " +  SERVER_URL + " and try again. Else contact ThingPulse Support and provide the device MAC address: " + WiFi.macAddress());
          } else {
            Serial.println("Device registration successful, now fetching OTP screen");
            DEVICE_ID = d.deviceId;
            DEVICE_SECRET = d.deviceSecret;
            saveConfig();
            parser.setDeviceSecret(DEVICE_SECRET);
            fetchAndDrawScreen(&parser);
          }
        }
      } else {
        showMessage("Failed to update time from internet (NTP). Please retry or verify your settings. " + CONFIG_MODE_INSTRUCTION);
      }
    } else {
      showMessage("Failed to connect to WiFi '" + WIFI_SSID + "'. Please retry or verify your settings. " + CONFIG_MODE_INSTRUCTION);
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
    EspaperParser parser(&gfx, SERVER_URL, DEVICE_SECRET, String(CLIENT_VERSION));
    fetchAndDrawScreen(&parser);
  }
  delay(100);
#endif
}
