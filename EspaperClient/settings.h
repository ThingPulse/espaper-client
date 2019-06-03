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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <FS.h>

/***************************
 * Device Type
 **************************/

// enable one of these devices for the Arduino IDE
// If you are using platformio enable the right device 
// in platformio.ini
//#define ESPAPER29BW
//#define ESPAPER42BW
//#define TTGOT529BW

#if defined(ESPAPER29BW)
  #define EPD29

  #define CS 15  // D8
  #define RST 2  // D4
  #define DC 5   // D1
  #define BUSY 4 // D2
  #define USR_BTN 12 // D6

  #define DEVICE_TYPE Espaper29Bw
#elif defined(ESPAPER42BW)
  #define EPD42

  #define CS 15  // D8
  #define RST 2  // D4
  #define DC 5   // D1
  #define BUSY 4 // D2
  #define USR_BTN 12 // D6
  
  #define DEVICE_TYPE Espaper42Bw
#elif defined(ESPAPER75BW)
  #define EPD75

  #define CS 2
  #define RST 15
  #define DC 5
  #define BUSY 4
  #define USR_BTN 12
  #define IMU_SDA 21
  #define IMU_SCL 22
  #define WAKE_UP_PIN GPIO_NUM_27
  #define BUZZER_PIN GPIO_NUM_32
  #define LED_PIN 26
  
  #define DEVICE_TYPE Espaper75Bw
#elif defined(ESP_COLOR_KIT)
  #define COLOR_TFT_24

  #define TFT_CS D1
  #define TFT_DC D2
  #define TFT_LED D8
  #define USR_BTN D4
  
  #define DEVICE_TYPE Espaper42Bw
#elif defined(TTGOT529BW)
  #define EPD29

  #define CS 5  // D8
  #define RST 12  // D4
  #define DC 19   // D1
  #define BUSY 4 // D2
  #define USR_BTN 37 // D6

  #define DEVICE_TYPE TTGOT529Bw
#endif


/***************************
 * User Settings
 **************************/

static String WIFI_SSID = "";
static String WIFI_PASS = "";
static String TIMEZONE = "";
static String NTP_SERVERS = "0.pool.ntp.org,1.pool.ntp.org,2.pool.ntp.org";

static String DEVICE_ID = "";
static String DEVICE_SECRET = "";

static uint8_t UPDATE_INTERVAL_MINS = 20;


/***************************
 * Internal Settings
 **************************/

//#define CLIENT_VERSION V027


//#define DEV_ENV
//#define TEST_ENV

const String CONFIG_SSID = "ESPaperConfig";
const String CONFIG_MODE_INSTRUCTION = "Press and hold LEFT button and press & release RIGHT button to enter configuration mode.";

// August 1st, 2018
#define NTP_MIN_VALID_EPOCH 1533081600
#define NTP_SYNC_TIMEOUT_SECONDS 5

#if defined(EPD29)
  const float MAX_TEXT_WIDTH_FACTOR = 0.95;
  const uint8_t STD_MESSAGE_Y_POSITION = 12;
  #define SCREEN_TYPE EPD29
#elif defined(EPD42)
  const float MAX_TEXT_WIDTH_FACTOR = 0.85;
  const uint8_t STD_MESSAGE_Y_POSITION = 25;
  #define SCREEN_TYPE EPD42
#elif defined(EPD75)
  const float MAX_TEXT_WIDTH_FACTOR = 0.75;
  const uint8_t STD_MESSAGE_Y_POSITION = 40;
  #define SCREEN_TYPE EPD75
#elif defined(COLOR_TFT_24)
  const float MAX_TEXT_WIDTH_FACTOR = 0.75;
  const uint8_t STD_MESSAGE_Y_POSITION = 40;
  #define SCREEN_TYPE COLOR_TFT_24
#endif

/**********************************
 * ESPaper Server-related Settings
 *********************************/

const String SERVER_API_DEVICES_PATH = "/public/devices";

#ifdef DEV_ENV
  // use empty array as a placeholder, as the scheme is HTTP rather
  // than HTTPS it won't actually be used, see EspaperParser::createWifiClient
  static const char rootCaCert[] PROGMEM = {};
  const String SERVER_URL = "http://192.168.0.146:8080";
  #define USE_SECURE_WIFI_CLIENT 0
#else
  #define USE_SECURE_WIFI_CLIENT 1
  // exported from Firefox as x509.pem format
  static const char rootCaCert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDSjCCAjKgAwIBAgIQRK+wgNajJ7qJMDmGLvhAazANBgkqhkiG9w0BAQUFADA/
MSQwIgYDVQQKExtEaWdpdGFsIFNpZ25hdHVyZSBUcnVzdCBDby4xFzAVBgNVBAMT
DkRTVCBSb290IENBIFgzMB4XDTAwMDkzMDIxMTIxOVoXDTIxMDkzMDE0MDExNVow
PzEkMCIGA1UEChMbRGlnaXRhbCBTaWduYXR1cmUgVHJ1c3QgQ28uMRcwFQYDVQQD
Ew5EU1QgUm9vdCBDQSBYMzCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AN+v6ZdQCINXtMxiZfaQguzH0yxrMMpb7NnDfcdAwRgUi+DoM3ZJKuM/IUmTrE4O
rz5Iy2Xu/NMhD2XSKtkyj4zl93ewEnu1lcCJo6m67XMuegwGMoOifooUMM0RoOEq
OLl5CjH9UL2AZd+3UWODyOKIYepLYYHsUmu5ouJLGiifSKOeDNoJjj4XLh7dIN9b
xiqKqy69cK3FCxolkHRyxXtqqzTWMIn/5WgTe1QLyNau7Fqckh49ZLOMxt+/yUFw
7BZy1SbsOFU5Q9D8/RhcQPGX69Wam40dutolucbY38EVAjqr2m7xPi71XAicPNaD
aeQQmxkqtilX4+U9m5/wAl0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNV
HQ8BAf8EBAMCAQYwHQYDVR0OBBYEFMSnsaR7LHH62+FLkHX/xBVghYkQMA0GCSqG
SIb3DQEBBQUAA4IBAQCjGiybFwBcqR7uKGY3Or+Dxz9LwwmglSBd49lZRNI+DT69
ikugdB/OEIKcdBodfpga3csTS7MgROSR6cz8faXbauX+5v3gTt23ADq1cEmv8uXr
AvHRAosZy5Q6XkjEGB5YGV8eAlrwDPGxrancWYaLbumR9YbK+rlmM6pZW87ipxZz
R8srzJmwN0jP41ZL9c8PDHIyh8bwRLtTcm1D9SZImlJnt1ir/md2cXjbDaJWFBM5
JDGFoqgCWjBH4d1QB7wCCZAA62RjYJsWvIjJEubSfZGL+T0yjWW06XyxV3bqxbYo
Ob8VZRzI9neWagqNdwvYkQsEjgfbKbYK7p2CNTUQ
-----END CERTIFICATE-----
)EOF";
    #ifdef TEST_ENV
      const String SERVER_URL = "https://test.espaper.com";

    #else
      // PROD
      const String SERVER_URL = "https://www.espaper.com";
    #endif

#endif

/***************************
 * Functions
 **************************/

static bool isDeviceRegistered() {
  return DEVICE_ID.length() != 0 && DEVICE_SECRET.length() != 0;
}

static void resetUserSettings() {
  WIFI_SSID = "";
  WIFI_PASS = "";
  UPDATE_INTERVAL_MINS = 20;
  TIMEZONE = "";
  NTP_SERVERS = "0.pool.ntp.org,1.pool.ntp.org,2.pool.ntp.org";
  DEVICE_ID = "";
  DEVICE_SECRET = "";
}

#endif //SETTINGS_H
