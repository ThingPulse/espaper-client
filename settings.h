/**The MIT License (MIT)
Copyright (c) 2018 by Daniel Eichhorn
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
See more at http://blog.squix.ch
*/

#define CLIENT_VERSION "V010"

//#define DEV_ENV
//#define TEST_ENV

// Config mode SSID
const String CONFIG_SSID = "ESPaperConfig";

// Setup
String WIFI_SSID = "";
String WIFI_PASS = "";

String DEVICE_ID = "";
String DEVICE_KEY = "";

const int UPDATE_INTERVAL_SECS = 20 * 60; // Update every 10 minutes

/*
 * BUSY---->gpio4 
 * RST---->gpio2 
 * DC---->gpio5 
 * CS---->gpio15 
 * CLK---->gpio14 
 * DIN---->gpio13
 * Buttons : Reset ( RST pins on esp ) , 
 * Flash ( GPIO-0 10K pull up ) 
 * User button ( GPIO-12 10K pull up )
 */
 /*
 Connect the following pins:
 Display  NodeMCU
 BUSY     D1
 RST      D2
 DC       D8
 CS       D3
 CLK      D5
 DIN      D7
 GND      GND
 3.3V     3V3
*/
/*
 * BUSY>gpio4 RST>gpio2 DC>gpio5 CS>gpio15 CLK>gpio14 DIN>gpio13
 */
#define CS 15  // D8
#define RST 2  // D4
#define DC 5   // D1
#define BUSY 4 // D2
#define USR_BTN 12 // D6

const String REQUEST_PATH = "/public/devices/";


#ifdef DEV_ENV
  const String SHA1_FINGERPRINT = "";
  const String SERVER_URL = "http://192.168.0.119:8080";
#else
  #ifdef TEST_ENV
    const String SHA1_FINGERPRINT = "FA 9D 01 C6 53 EF 0D DA E8 E8 7F 57 43 61 07 05 DC 86 7E 70";
    const String SERVER_URL = "https://test.espaper.com";
  #else
    // PROD
    const String SHA1_FINGERPRINT = "3E 12 02 6B BE 41 EF A7 29 8C 1F 2C 25 3C 6D 21 7C 6C 83 37";
    const String SERVER_URL = "https://www.espaper.com";
  #endif
#endif

/***************************
 * End Settings
 **************************/

