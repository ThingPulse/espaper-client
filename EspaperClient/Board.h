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

#ifndef BOARD_H
#define BOARD_H

#include <Arduino.h>
#include "settings.h"

#if defined(ESP8266) 
    extern "C" {
    #include "user_interface.h"  // Required for wifi_station_connect() to work
    }
    #include <ESP8266WiFi.h>
    #include <ESP8266HTTPClient.h>
    #include <ESP8266httpUpdate.h>
    #include <FS.h>

    #define HTTP_UPDATER ESPhttpUpdate
#elif defined(ESP32)
    #include <WiFi.h>
    #include <HTTPClient.h>
    #include <HTTPUpdate.h>
    #include <SPIFFS.h>

    #define HTTP_UPDATER httpUpdate
#endif

class BoardClass {

    public:
        BoardClass();

        String getChipId();

        void deepSleep(uint32_t sleepSeconds);

        boolean connectWifi(String wifiSsid, String wifiPassword);

        static void sleepWifi();

        uint32_t getBattery();

        uint32_t getFreeSPIFFSBytes();

        uint32_t getTotalSPIFFSBytes();

        WiFiClient* createWifiClient(const char *rootCertificate);

};

extern BoardClass Board;

#endif //BOARD_H