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

// https://www.bakke.online/index.php/2017/06/24/esp8266-wifi-power-reduction-avoiding-network-scan/
// The ESP8266 RTC memory is arranged into blocks of 4 bytes. The access methods read and write
// 4 bytes at a time, so the RTC data structure should be padded to a 4-byte multiple.
struct {
  uint32_t crc32;     // 4 bytes
  uint8_t channel;    // 1 byte,   5 in total
  uint8_t bssid[6];   // 6 bytes, 11 in total
  uint8_t padding;    // 1 byte,  12 in total
} rtcData;

BoardClass Board;

#if defined(ACTIVITY_LED_PIN)
  volatile byte ledState;
  volatile long timerCalls;
  volatile long lastBlinkCall;
#endif

BoardClass::BoardClass() {

}

boolean BoardClass::init() {
  #if defined(USR_BTN)
    pinMode(USR_BTN, INPUT_PULLUP);
  #endif

  #if defined(IMU_ADXL345)
    if (!accelerometer.begin(IMU_SDA, IMU_SCL))
    {
      Serial.println("Could not find a valid ADXL345 sensor, check wiring!");
      delay(500);
    }


    // Set tap detection on Z-Axis
    accelerometer.setTapDetectionX(0);       // Don't check tap on X-Axis
    accelerometer.setTapDetectionY(0);       // Don't check tap on Y-Axis
    accelerometer.setTapDetectionZ(1);       // Check tap on Z-Axis
    // or
    // accelerometer.setTapDetectionXYZ(1);  // Check tap on X,Y,Z-Axis

    accelerometer.setTapThreshold(2.5);      // Recommended 2.5 g
    accelerometer.setTapDuration(0.02);      // Recommended 0.02 s
    accelerometer.setDoubleTapLatency(0.10); // Recommended 0.10 s
    accelerometer.setDoubleTapWindow(0.30);  // Recommended 0.30 s

    accelerometer.setActivityThreshold(2.0);    // Recommended 2 g
    accelerometer.setInactivityThreshold(2.0);  // Recommended 2 g
    accelerometer.setTimeInactivity(5);         // Recommended 5 s

    // Set activity detection only on X,Y,Z-Axis
    //accelerometer.setActivityXYZ(1);         // Check activity on X,Y,Z-Axis
    // or
    accelerometer.setActivityX(1);        // Check activity on X_Axis
    accelerometer.setActivityY(1);        // Check activity on Y-Axis
    accelerometer.setActivityZ(0);        // Check activity on Z-Axis

    // Set inactivity detection only on X,Y,Z-Axis
    //accelerometer.setInactivityXYZ(1);       // Check inactivity on X,Y,Z-Axis

    // Select INT 1 for get activities
    accelerometer.useInterrupt(ADXL345_INT1);
    pinMode(WAKE_UP_PIN, INPUT);

    esp_sleep_enable_ext0_wakeup(WAKE_UP_PIN,1);
    attachInterrupt(digitalPinToInterrupt(WAKE_UP_PIN), &BoardClass::wakeup, CHANGE);
  #endif
  return true;

}

void BoardClass::wakeup() {
  Serial.println("Triggered wakeup");
  ESP.restart();
}

boolean BoardClass::isConfigMode() {
  Serial.println(F("Checking config mode"));
  #if defined(IMU_ADXL345)
    if (getRotation() == 4) {
      Serial.println(F("Device laying flat. Entering config mode..."));
      return true;
    }
    return false;
  #endif

  #if defined(USR_BTN)
    return !digitalRead(USR_BTN);
  #endif
}

uint8_t BoardClass::getRotation() {
  #if defined(IMU_ADXL345)
    uint8_t sampleCount = 5;
    Vector norm = accelerometer.readNormalize();
    // We need to read activities to flush FIFO buffer
    Activites activites = accelerometer.readActivites();
    uint8_t rotation = 0;
    uint8_t unchangedRotationCount = 0;
    uint8_t currentRotation = 0;
    while (true) {
      if (norm.ZAxis > 8) {
        return 4;
      } else if (norm.XAxis > 8) {
        currentRotation = 1;
      } else if (norm.XAxis < -8) {
        currentRotation = 3;
      } else if (norm.YAxis > 8) {
        currentRotation = 2;
      } else if (norm.YAxis < -8) {
        currentRotation = 0;
      } else {
        currentRotation =  3;
      }
      if (rotation == currentRotation) {
        unchangedRotationCount++;
      } else {
        rotation = currentRotation;
        unchangedRotationCount = 0;
      }

      if (unchangedRotationCount > sampleCount) {
        //Serial.printf_P(FPSTR("Rotation: %d"), rotation);
        return rotation;
      }
      delay (100);
    }
    return 0;
  #else
    return DEVICE_ROTATION;
  #endif
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
    // Take 80% of maxDeepSleep, since we suspect it to be too optimistic in some
    // cases This would cause some devices to never wake up again on their own.
    uint64_t maxSleepMicroSeconds = ESP.deepSleepMax() * 0.8;

    uint64_t calculatedSleepMicroSeconds = maxSleepMicroSeconds < sleepMicroSeconds ? maxSleepMicroSeconds : sleepMicroSeconds;
    Serial.printf_P(PSTR("Going to sleep for: %d[s]\n"), calculatedSleepMicroSeconds / 1000000);
    ESP.deepSleep(calculatedSleepMicroSeconds, WAKE_RF_DISABLED);
  #elif defined(ESP32)
    Serial.printf_P(PSTR("Going to sleep for: %d[s]\n"), sleepSeconds);
    // https://docs.espressif.com/projects/esp-idf/en/latest/api-reference/system/sleep_modes.html#wifi-bt-and-sleep-modes
    esp_wifi_stop();
    esp_bt_controller_disable();
    esp_sleep_enable_timer_wakeup(sleepSeconds * 1000000);
    esp_deep_sleep_start();
  #endif
}

boolean BoardClass::connectWifi(String wifiSsid, String wifiPassword) {
  Serial.println("(Re-)Connecting to WiFi.");
  // Wake up WiFi
  #if defined(ESP8266)
    WiFi.forceSleepWake();
    delay(1);
  #elif defined(ESP32)

  #endif

  // Disable the WiFi persistence.  The ESP8266 will not load and save WiFi settings in the flash memory.
  // https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-1/
  WiFi.persistent(false);
  WiFi.mode(WIFI_STA);

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("Already connected; doing nothing.");
    return true;
  }

  // this 3 lines for a fix IP-address (and faster connection)
  /*IPAddress ip(192, 168, 0, 60);
  IPAddress gateway(192, 168, 0, 1);
  IPAddress subnet(255, 255, 255, 0);
  IPAddress dns(8, 8, 8, 8);
  WiFi.config (ip, gateway, subnet, dns);*/

  // Try to read WiFi settings from RTC memory
  bool rtcValid = readRtcData();

  const char* pwd = wifiPassword.c_str();
  if (wifiPassword == "") {
    Serial.println("Using SSID without password.");
    pwd = nullptr;
  }

  if (rtcValid) {
    Serial.println("Found valid WiFi quick-connect settings in RTC (channel & BSSID). Applying all.");
    WiFi.begin(wifiSsid.c_str(), pwd, rtcData.channel, rtcData.bssid, true);
  } else {
    Serial.println("Setting simple WiFi config (w/o channel & BSSID).");
    WiFi.begin(wifiSsid.c_str(), pwd);
  }

  int i = 0;
  Serial.print("Establishing connection to AP");
  uint32_t startTime = millis();
  Serial.print("WiFi connect");
  while (WiFi.status() != WL_CONNECTED) {
    i++;
    if (i == 100 && rtcValid) {
      Serial.println("\nQuick-connect failed. Resetting WiFi for a second attempt w/o preset channel & BSSID.");
      sleepWifi();
      WiFi.mode(WIFI_STA);
      delay(10);
      WiFi.begin(wifiSsid.c_str(), pwd);
    }
    if (i == 300) {
      Serial.println("\nConnection could not be established in ~30s. Giving up.");
      return false;
    }
    Serial.print(".");
    delay(100);
  }
  Serial.printf("\nConnected after %dms. Using IP %s.\n", (millis() - startTime), WiFi.localIP().toString().c_str());

  persistToRtc();
  return true;
}

void BoardClass::sleepWifi() {
  Serial.println("Disconnecting WiFi and turning modem off.");
  WiFi.disconnect();
  // https://github.com/esp8266/Arduino/issues/4082
  delay(200);
  WiFi.mode(WIFI_OFF);

  #if defined(ESP8266)
    delay(10);
    WiFi.forceSleepBegin();
  #elif defined(ESP32)

  #endif
  delay(100);
}

uint32_t BoardClass::getBattery() {
  #if defined(ESP8266)
    return analogRead(A0);
  #elif defined(ESP32)
    // n/a, see https://github.com/espressif/arduino-esp32/issues/469

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

uint32_t BoardClass::calculateCRC32(const uint8_t *data, size_t length) {
  uint32_t crc = 0xffffffff;
  while (length--) {
    uint8_t c = *data++;
    for (uint32_t i = 0x80; i > 0; i >>= 1) {
      bool bit = crc & 0x80000000;
      if (c & i) {
        bit = !bit;
      }
      crc <<= 1;
      if (bit) {
        crc ^= 0x04c11db7;
      }
    }
  }
  return crc;
}

// Try to read WiFi settings from RTC memory. Return true if successful; false otherwise.
bool BoardClass::readRtcData() {
  bool rtcValid = false;
  #if defined(ESP8266)
    if (ESP.rtcUserMemoryRead(0, (uint32_t *)&rtcData, sizeof(rtcData))) {
      // Calculate the CRC of what we just read from RTC memory, but skip the
      // first 4 bytes as that's the checksum itself.
      uint32_t crc = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
      if (crc == rtcData.crc32) {
        rtcValid = true;
      }
    }
  #elif defined(ESP32)

  #endif
  return rtcValid;
}

// Write current connection info back to RTC.
void BoardClass::persistToRtc() {
  rtcData.channel = WiFi.channel();
  memcpy(rtcData.bssid, WiFi.BSSID(), 6); // Copy 6 bytes of BSSID (AP's MAC address)
  rtcData.crc32 = calculateCRC32(((uint8_t *)&rtcData) + 4, sizeof(rtcData) - 4);
  #if defined(ESP8266)
    ESP.rtcUserMemoryWrite(0, (uint32_t *)&rtcData, sizeof(rtcData));
  #elif defined(ESP32)

  #endif
}
