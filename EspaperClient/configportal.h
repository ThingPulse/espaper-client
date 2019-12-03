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

#ifndef CONFIG_PORTAL_H
#define CONFIG_PORTAL_H

#if defined(ESP8266) 
 #include <ESP8266WebServer.h>
#elif defined(ESP32)
  #include <WebServer.h>
#endif

#include <MiniGrafx.h>
#include "Board.h"
#include "artwork.h"
#include "timezones.h"

const String CONFIG_FILE = "/espaper.txt";
const String DATA_FILE = "/device-data.txt";

const char HTTP_HEAD_START[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>"; // <link rel=\"icon\" type=\"image/png\" sizes=\"16x16\" href=\"/favicon.png\">
const char HTTP_STYLE[] PROGMEM           = "<style>div,input, select{padding:5px;font-size:1em;} input, select{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>"
                                            "document.addEventListener('DOMContentLoaded',function() {"
                                            "    document.getElementById('ssid').focus();"
                                            "    document.getElementById('timeZone').onchange = setNtpServers;"
                                            "    document.getElementsByTagName('h2')[1].onclick = function() { document.getElementById('proxyConfig').style.display = ''; };"
                                            "}, false);"
                                            "function setNtpServers() {"
                                            "    const timeZone = document.getElementById('timeZone').value;"
                                            "    const region = timeZone.substring(0, timeZone.indexOf('/'));"
                                            "    let base = 'pool.ntp.org';"
                                            "    if (region) {"
                                            "        switch (region) {"
                                            "            case 'Africa':"
                                            "            case 'Antarctica':"
                                            "            case 'Asia':"
                                            "            case 'Europe':"
                                            "                base = region.toLowerCase() + '.' + base;"
                                            "                break;"
                                            "            case 'Australia':"
                                            "            case 'Pacific':"
                                            "                base = 'oceania.' + base;"
                                            "                break;"
                                            "            case 'Brazil':"
                                            "            case 'Chile':"
                                            "            case 'Mexico':"
                                            "                base = 'south-america.' + base;"
                                            "                break;"
                                            "            case 'Canada':"
                                            "            case 'US':"
                                            "                base = 'north-america.' + base;"
                                            "                break;"
                                            "        }"
                                            "    }"
                                            "    document.getElementById('ntpServers').value = '0.' + base + ',' + '1.' + base + ',' + '2.' + base;"
                                            "}"
                                            "function confirmDelete() {"
                                            "    return confirm('Resetting the configuration will also delete the credentials with which the device authenticates against the ESPaper server. Your device will still be showing in the portal but it will no longer be able to connect. It has to be registered again after which you will see it as a new device on the server.');"
                                            "}"
                                            "</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='post' action='save'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<label for='{i}'>{p}</label><br/><input id='{i}' name='{n}' maxlength={l}  value='{v}' {c}><br/><br/>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>Save Configuration</button></form><br/><form action=\"/delete\" method=\"get\" onsubmit=\"return confirmDelete();\"><button>Reset Firmware Settings</button></form><br/><form action=\"/reset\" method=\"get\"><button>Restart ESPaper</button></form>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";
const char HTTP_OPTION_ITEM[] PROGMEM     = "<option value=\"{v}\" {s}>{n}</option>";
const char HTTP_LOGO[] PROGMEM            = "<img src=\"/logo.svg\" width=\"400px\" />";

#define ACTION_AFTER_REBOOT_UPDATE_SCREEN 0
#define ACTION_AFTER_REBOOT_UPGRADE_FIRMWARE 10

typedef struct DeviceData {
  uint16_t totalDeviceStarts;
  uint16_t successfulDeviceStarts;
  time_t lastNtpSyncTime;
  uint8_t startsWithoutNtpSync;
  uint32_t lastCycleDuration;
  uint8_t actionAfterReboot;
} DeviceData;


#if defined(ESP8266) 
  ESP8266WebServer server (80);
#elif defined(ESP32)
  WebServer server (80);
#endif

String getFormField(String id, String placeholder, String length, String value, String customHTML) {
  String pitem = FPSTR(HTTP_FORM_PARAM);

  pitem.replace("{i}", id);
  pitem.replace("{n}", id);
  pitem.replace("{p}", placeholder);
  pitem.replace("{l}", length);
  pitem.replace("{v}", value);
  pitem.replace("{c}", customHTML);
  return pitem;
}


boolean saveConfig() {
  File f = SPIFFS.open(CONFIG_FILE, "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  f.print("WIFI_SSID=");
  f.println(WIFI_SSID);
  f.print("WIFI_PASS=");
  f.println(WIFI_PASS);
  f.print("UPDATE_INTERVAL_MINS=");
  f.println(UPDATE_INTERVAL_MINS);
  f.print("TIMEZONE=");
  f.println(TIMEZONE);
  f.print("NTP_SERVERS=");
  f.println(NTP_SERVERS);
  f.print("DEVICE_ID=");
  f.println(DEVICE_ID);
  f.print("DEVICE_SECRET=");
  f.println(DEVICE_SECRET);
  f.flush();
  f.close();
  Serial.println("Saved values");
  return true;
}

boolean loadConfig() {
  File f = SPIFFS.open(CONFIG_FILE, "r");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  while (f.available()) {
    String key = f.readStringUntil('=');
    String value = f.readStringUntil('\r');
    f.read();
    Serial.println(key + " = [" + value + "]");
    if (key == "WIFI_SSID") {
      WIFI_SSID = value;
    }
    if (key == "WIFI_PASS") {
      WIFI_PASS = value;
    }
    if (key == "UPDATE_INTERVAL_MINS") {
      UPDATE_INTERVAL_MINS = value.toInt();
    }
    if (key == "TIMEZONE") {
      TIMEZONE = value;
    }
    if (key == "NTP_SERVERS") {
      NTP_SERVERS = value;
    }
    if (key == "DEVICE_ID") {
      DEVICE_ID = value;
    }
    if (key == "DEVICE_SECRET" || key == "DEVICE_KEY") { // support pre-V011 clients
      DEVICE_SECRET = value;
    }
    // break infinite loop if file is present but somehow empty or corrupt
    if (key == "" && value == "") {
      Serial.println(F("Config file available on SPIFFS but at least one line with empty key and value found. Exiting read loop."));
      break;
    }
  }
  if (WIFI_SSID == "" || WIFI_PASS == "" || TIMEZONE == "") {
    Serial.println("At least one configuration property not yet defined");
    return false;
  }
  f.flush();
  f.close();
  Serial.println("Loaded config");
  return true;
}



void loadDeviceData(DeviceData *deviceData) {
  File f = SPIFFS.open(DATA_FILE, "r");
  if (!f) {
    Serial.println(F("Failed to open data file. Initializing struct."));
    
    deviceData->totalDeviceStarts = 0;
    deviceData->successfulDeviceStarts = 0;
    deviceData->lastNtpSyncTime = 0;
    deviceData->startsWithoutNtpSync = 0;
    deviceData->lastCycleDuration = 0;
    deviceData->actionAfterReboot = ACTION_AFTER_REBOOT_UPDATE_SCREEN;
    return;
  }
  while (f.available()) {
    String key = f.readStringUntil('=');
    String value = f.readStringUntil('\r');
    f.read();
    Serial.println(key + " = [" + value + "]");
    if (key == "TOTAL_DEVICE_STARTS") {
      deviceData->totalDeviceStarts = value.toInt();
    }
    if (key == "SUCCESSFUL_DEVICE_STARTS") {
      deviceData->successfulDeviceStarts = value.toInt();
    }
    if (key == "LAST_NTP_SYNC_TIME") {
      deviceData->lastNtpSyncTime = value.toInt();
    }
    if (key == "STARTS_WITHOUT_NTP_SYNC") {
      deviceData->startsWithoutNtpSync = value.toInt();
    }
    if (key == "LAST_CYCLE_DURATION") {
      deviceData->lastCycleDuration = value.toInt();
    }
    if (key == "ACTION_AFTER_REBOOT") {
      deviceData->actionAfterReboot = value.toInt();
    }
    // break infinite loop if file is present but somehow empty or corrupt
    if (key == "" && value == "") {
      Serial.println(F("Device data file available on SPIFFS but at least one line with empty key and value found. Exiting read loop."));
      break;
    }
  }
  f.flush();
  f.close();
  Serial.println(F("Loaded data file"));
}

void saveDeviceData(DeviceData* deviceData) {
  File f = SPIFFS.open(DATA_FILE, "w+");
  if (!f) {
    Serial.println(F("Failed to open data file"));
    return;
  }
  f.print(F("TOTAL_DEVICE_STARTS="));
  f.println(deviceData->totalDeviceStarts);
  f.print(F("SUCCESSFUL_DEVICE_STARTS="));
  f.println(deviceData->successfulDeviceStarts);
  f.print(F("LAST_NTP_SYNC_TIME="));
  f.println(deviceData->lastNtpSyncTime);
  f.print(F("STARTS_WITHOUT_NTP_SYNC="));
  f.println(deviceData->startsWithoutNtpSync);
  f.print(F("LAST_CYCLE_DURATION="));
  f.println(deviceData->lastCycleDuration);
  f.print(F("ACTION_AFTER_REBOOT="));
  f.println(deviceData->actionAfterReboot);
  f.flush();
  f.close();
  Serial.println(F("Saved values in data file."));
}

void handleRoot() {
  Serial.println("Serving /");
  server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
  server.sendHeader("Pragma", "no-cache");
  server.sendHeader("Expires", "-1");
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  String header = FPSTR(HTTP_HEAD_START);
  header.replace("{v}", "Options");
  server.sendContent(header);
  server.sendContent(FPSTR(HTTP_SCRIPT));
  server.sendContent(FPSTR(HTTP_STYLE));
  server.sendContent(FPSTR(HTTP_HEAD_END));
  server.sendContent(FPSTR(HTTP_LOGO));
  server.sendContent("<h1>ESPaper Configuration</h1>");
  server.sendContent("<h2>Device Information</h2>");
  server.sendContent("WiFi MAC address: ");
  server.sendContent(WiFi.macAddress());
  server.sendContent("<BR/>Free Heap: ");
  server.sendContent(String(ESP.getFreeHeap() / 1024.0));
  server.sendContent(" KB<BR/>Chip  Id: ");
  server.sendContent(Board.getChipId());
  server.sendContent("<BR/>Flash Chip Size: ");
  server.sendContent(String(ESP.getFlashChipSize() / (1024 * 1024))); // no floating point required, it's always n MB
  server.sendContent(" MB<h2>Settings</h2>");
  server.sendContent(FPSTR(HTTP_FORM_START));
  server.sendContent(getFormField("ssid", "WiFi SSID", "32", WIFI_SSID, ""));
  server.sendContent(getFormField("password", "WiFi Password", "63", WIFI_PASS, "type='password'"));
  // 3h = 180min seem to be a save value. Since ESP.deepSleepMax() returns a different value every time validation
  // based on a dynamic value can be very irritating for the user.
  String validationRules = String("type='number' min='1' max='180'");
  server.sendContent(getFormField("updateIntervalMins", "Screen Update Interval in Minutes", "10", String(UPDATE_INTERVAL_MINS), validationRules));
  server.sendContent("<label for=\"timeZone\">Time Zone</label>");
  server.sendContent("<select id=\"timeZone\" name=\"timeZone\">");
  char tzid[50];
  char tzcode[50];
  bool isParsingTzid = true;
  uint8_t charCounter = 0;
  for (int i = 0; i < timezones_txt_len; i++) {
    char nextChar = pgm_read_byte(timezones_txt + i);
    if (nextChar == ' ') {
      tzid[charCounter] = '\0';
      charCounter = 0;
      isParsingTzid = false;
    } else if (nextChar == '\n') {
      tzcode[charCounter] = '\0';
      charCounter = 0;
      isParsingTzid = true;
      String option = FPSTR(HTTP_OPTION_ITEM);
      String timeZoneId = String(tzid);
      String timeZoneCode = String(tzcode);
      String timeZone = timeZoneId + " " + timeZoneCode;

      timeZoneId.replace("_", " ");
      option.replace("{v}", timeZone);
      option.replace("{n}", timeZoneId);
      if (TIMEZONE == timeZone) {
        option.replace("{s}", "selected");
      } else {
        option.replace("{s}", "");
      }
      server.sendContent(option);
    } else if (isParsingTzid) {
      tzid[charCounter] = nextChar;
      charCounter++;
    } else {
      tzcode[charCounter] = nextChar;
      charCounter++;
    }

  }
  server.sendContent("</select>");
  server.sendContent("<br><br>");
  server.sendContent(getFormField("ntpServers", "NTP Servers", "300", NTP_SERVERS, ""));
  server.sendContent(FPSTR(HTTP_FORM_END));
  server.sendContent(FPSTR(HTTP_END));
  server.sendContent("");
  server.client().stop();
}

void sendRedirectToRoot() {
  server.sendHeader("Location", "/");
  server.send(303, "text/html", "");
}

void handleDelete() {
  Serial.println("Handling 'delete' request");
  SPIFFS.remove(CONFIG_FILE);
  resetUserSettings();
  sendRedirectToRoot();
}

void handleReset() {
  pinMode(2, INPUT);
  pinMode(0, INPUT);
  ESP.restart();  
}

void handleSave() {
  Serial.println("Handling 'save' request");
  WIFI_SSID = server.arg("ssid");
  WIFI_PASS = server.arg("password");
  UPDATE_INTERVAL_MINS = server.arg("updateIntervalMins").toInt();
  TIMEZONE = server.arg("timeZone");
  NTP_SERVERS = server.arg("ntpServers");
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASS);
  Serial.println(UPDATE_INTERVAL_MINS);
  Serial.println(TIMEZONE);
  Serial.println(NTP_SERVERS);
  saveConfig();
  sendRedirectToRoot();
}

void handleNotFound() {
  Serial.println("Handling HTTP 404");
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += ( server.method() == HTTP_GET ) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";

  for ( uint8_t i = 0; i < server.args(); i++ ) {
    message += " " + server.argName ( i ) + ": " + server.arg ( i ) + "\n";
  }

  server.send(404, "text/plain", message);
}

void registerServerCallbackHandlers() {
  server.on("/", handleRoot);
  server.on("/logo.svg", []() {
    Serial.println("Serving /logo.svg");
    server.setContentLength(ThingPulse_svg_len);
    server.send(200, "image/svg+xml", "");
    for (int i = 0; i < ThingPulse_svg_len; i++) {
      char svgChar = pgm_read_byte(ThingPulse_svg + i);
      server.sendContent(String(svgChar));
    }
    server.sendContent("");
    server.client().stop();
  } );
  server.on("/favicon.png", []() {
    // TODO don't understand why this doesn't work :(
    Serial.println("Serving /favicon.png");
    server.setContentLength(favicon_png_len);
    server.send(200, "image/png", "");
    for (int k = 0; k < favicon_png_len; k++) {
      char pngChar = pgm_read_byte(favicon_png + k);
      server.sendContent(String(pngChar));
    }
    server.sendContent("");
    server.client().stop();
  } );
  server.on("/save", handleSave);
  server.on("/delete", handleDelete);
  server.on("/reset", handleReset);
  server.onNotFound(handleNotFound);  
}

String getJoinWifiMessage() {
  return "Join WiFi with SSID '" + CONFIG_SSID + "' then open browser at http://" + WiFi.softAPIP().toString();  
}

void startConfigPortal(MiniGrafx *gfx) {
  Serial.println("Starting config portal...");

  WiFi.mode(WIFI_AP);
  WiFi.softAP(CONFIG_SSID.c_str());

  gfx->init();  
  gfx->fillBuffer(1);
  gfx->drawPalettedBitmapFromPgm(BOOT_SCREEN_X, BOOT_SCREEN_Y, boot_screen);
  gfx->setColor(0);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);

  File f = SPIFFS.open(CONFIG_FILE, "r");
  uint16_t halfWidth = gfx->getWidth() / 2;
  uint16_t maxTextWidth = gfx->getWidth() * MAX_TEXT_WIDTH_FACTOR;
  if (f) {
    Serial.println("Configuration file present -> assume user manually started config portal");
    gfx->setFont(ArialMT_Plain_16);
    gfx->drawString(halfWidth, JOIN_WIFI_MESSAGE_Y, "ESPaper Configuration Mode");
    gfx->drawStringMaxWidth(halfWidth, JOIN_WIFI_MESSAGE_Y + 19, maxTextWidth, getJoinWifiMessage());
  } else {
    Serial.println("Configuration file doesn't exist or is not readable -> assume virgin device");
    #ifdef EPD29
      gfx->setFont(ArialMT_Plain_16);
      gfx->drawString(halfWidth, 39, "ESPaper Initial Setup");
      gfx->drawStringMaxWidth(halfWidth, 58, maxTextWidth, getJoinWifiMessage());
    #endif
    #ifdef EPD42
      gfx->setFont(ArialMT_Plain_24);
      gfx->drawString(halfWidth, 172, "ESPaper Initial Setup");
      gfx->setFont(ArialMT_Plain_16);
      gfx->drawStringMaxWidth(halfWidth, 202, maxTextWidth, getJoinWifiMessage());
    #endif
  }
  
  Serial.println("Committing screen");
  gfx->commit();
  gfx->freeBuffer();

  registerServerCallbackHandlers();
  server.begin();
  Serial.println("HTTP server started");

  while (true) {
    server.handleClient();
    yield();
  }
}

String getTimeZoneSettings() {
  uint8_t indexOfSeparator = TIMEZONE.indexOf(" ");
  if (indexOfSeparator <= 0) {
    return "";
  }
  return TIMEZONE.substring(indexOfSeparator + 1);
}

String getTimeZoneName() {
  uint8_t indexOfSeparator = TIMEZONE.indexOf(" ");
  if (indexOfSeparator <= 0) {
    return "";
  }
  return TIMEZONE.substring(0, indexOfSeparator);
}

String getNtpServer(int index) {
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = NTP_SERVERS.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (NTP_SERVERS.charAt(i) == ',' || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }

  return found > index ? NTP_SERVERS.substring(strIndex[0], strIndex[1]) : "";
}

void saveDeviceRegistration(String deviceId, String deviceSecret) {
  DEVICE_ID = deviceId;
  DEVICE_SECRET = deviceSecret;
  saveConfig();
}

#endif