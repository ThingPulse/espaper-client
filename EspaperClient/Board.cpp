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

#include "Board.h"



BoardClass Board;

BoardClass::BoardClass() {

}

String BoardClass::getChipId() {
        
    #if defined(ESP8266) 
        return String(ESP.getChipId());
    #elif defined(ESP32)
        uint64_t chipId = ESP.getEfuseMac();
        uint32_t low = chipId % 0xFFFFFFFF; 
        uint32_t high = (chipId >> 32) % 0xFFFFFFFF;
        return String(low) + String(high);
    #endif
}

void BoardClass::deepSleep(uint64_t sleepSeconds) {
    #if defined(ESP8266) 
        uint64_t sleepMicroSeconds = sleepSeconds * 1000000;
        // Take 80% of maxDeepSleep, since we suspect it to be too optimistic in some cases
        // This would cause some devices to never wake up again on their own.
        uint64_t maxSleepMicroSeconds = ESP.deepSleepMax() * 0.8;

        uint64_t calculatedSleepMicroSeconds = maxSleepMicroSeconds < sleepMicroSeconds ? maxSleepMicroSeconds : sleepMicroSeconds;
        Serial.printf_P(PSTR("Going to sleep for: %d[s]\n"),  calculatedSleepMicroSeconds / 1000000);
        ESP.deepSleep(calculatedSleepMicroSeconds, WAKE_RF_DISABLED);
    #elif defined(ESP32)
        Serial.printf_P(PSTR("Going to sleep for: %d[s]\n"),  sleepSeconds);
        esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000);
        esp_deep_sleep_start();
    #endif
}

boolean BoardClass::connectWifi(String wifiSsid, String wifiPassword) {
  // Wake up WiFi
    #if defined(ESP8266) 
        wifi_fpm_do_wakeup();
        wifi_fpm_close();

        Serial.println("Reconnecting");
        wifi_set_opmode(STATION_MODE);
        wifi_station_connect();
    #elif defined(ESP32)

    #endif


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

  if (wifiPassword == "") {
    Serial.println("Only SSID without password");
    WiFi.begin(wifiSsid.c_str());
  } else {
    WiFi.begin(wifiSsid.c_str(), wifiPassword.c_str());
  }

  int i = 0;
  uint32_t startTime = millis();
  Serial.print("WiFi connect");
  while (WiFi.status() != WL_CONNECTED) {
    delay(100);
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

void BoardClass::sleepWifi() {

    // https://github.com/esp8266/Arduino/issues/4082
    delay(200);
    WiFi.mode(WIFI_OFF);
    
    #if defined(ESP8266) 
        WiFi.forceSleepBegin();
    #elif defined(ESP32)

    #endif
  delay(100);
}

uint32_t BoardClass::getBattery() {
    #if defined(ESP8266) 
        return analogRead(A0);
    #elif defined(ESP32)
        // actually for TTGO T5 V2.2 board, not ESP32 in general
        return analogRead(A7);
    #endif

}

uint32_t BoardClass::getFreeSPIFFSBytes() {

    #if defined(ESP8266) 
        FSInfo fs_info;
        SPIFFS.info(fs_info);
        return fs_info.totalBytes - fs_info.usedBytes;
    #elif defined(ESP32)
        return SPIFFS.totalBytes() - SPIFFS.usedBytes();
    #endif
}

uint32_t BoardClass::getTotalSPIFFSBytes() {
    #if defined(ESP8266) 
        FSInfo fs_info;
        SPIFFS.info(fs_info);
        return fs_info.totalBytes;
    #elif defined(ESP32)
        return SPIFFS.totalBytes();
    #endif
}

WiFiClient* BoardClass::createWifiClient(const char *rootCertificate) {
  // it would be correct to do this via
  // #ifdef DEV_ENV
  // as it's a compile-time configuration rather than a runtime configuration but including 
  // #include "settings.h" leads to odd compile errors
  #ifdef USE_SECURE_WIFI_CLIENT
    Serial.println("Using secure WiFi client");
    WiFiClientSecure *client = new WiFiClientSecure();
    Serial.println("[HTTP] configuring server root cert in client");
    #if defined(ESP8266) 
      client->setTrustAnchors(new X509List(rootCertificate));
      /*bool mfln = client->probeMaxFragmentLength(url.host, url.port, 1024); 
      Serial.printf("MFLN supported: %s\n", mfln ? "yes" : "no");
      if (mfln) {
        client->setBufferSizes(1024, 1024);
      }*/
    #elif defined(ESP32)
      client->setCACert(rootCertificate);
    #endif
    return client;
  #else
    Serial.println("Using non-secure WiFi client");
    return new WiFiClient();
  #endif
  
}