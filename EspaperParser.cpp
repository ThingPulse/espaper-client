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

#include "EspaperParser.h"

EspaperParser::EspaperParser(MiniGrafx *gfx, uint8_t screenWidth, String baseUrl, String deviceSecret, String clientVersion) {
  this->gfx = gfx;
  this->screenWidth = screenWidth;
  this->baseUrl = baseUrl;
  this->deviceSecret = deviceSecret;
  this->clientVersion = clientVersion;
}

void EspaperParser::setRootCertificate(const char *rootCertificate) {
  this->rootCert = rootCertificate;
}

void EspaperParser::setDeviceSecret(String deviceSecret) {
  this->deviceSecret = deviceSecret;  
}

EspaperParser::DeviceIdAndSecret EspaperParser::registerDevice(String requestPath, String jsonData) {
  DeviceIdAndSecret result;
  // set default values signifying an error condition
  result.deviceId = "-1";
  result.deviceSecret = "-2";
  
  Url url = this->dissectUrl(this->baseUrl + requestPath);

  WiFiClient client = this->createWifiClient(url.protocol);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  client.connect(url.host, url.port);
  if (!client.connected()) {
    Serial.println("*** Can't connect. ***\n-------");
    return result;
  }

  String EOL = "\r\n";

  String request = "POST " + url.path + " HTTP/1.1\r\n" +
                   "Host: " + url.host + "\r\n" +
                   "User-Agent: ESPaperClient/1.0\r\n" +
                   "Content-Type: application/json\r\n" +
                   "Content-Length: " + jsonData.length() + "\r\n" +
                   "X-ESPAPER-CLIENT-VERSION: " + this->clientVersion + EOL +
                   "X-ESPAPER-BATTERY: " + String(analogRead(A0)) + EOL +
                   "X-ESPAPER-WIFI-RSSI: " + String(WiFi.RSSI()) + EOL +
                   "Connection: close\r\n\r\n" + jsonData;

  Serial.println("Sending request: " + request);

  client.print(request);

  long lastUpdate = millis();

  int httpCode = 0;
  while (client.available() || client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);

    if (line.startsWith("HTTP/1.")) {
      httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
      Serial.printf("HTTP Code: %d\n", httpCode);
    }
    if (line.startsWith("X-ESPAPER-SECRET")) {
      result.deviceSecret = line.substring(18, line.indexOf('\r'));
    }
    if (line.startsWith("X-ESPAPER-DEVICE-ID")) {
      result.deviceId = line.substring(21, line.indexOf('\r'));
    }
    if (line == "\r" || line == "\r\n") {
      Serial.println("headers received");
      break;
    }
    if (millis() - lastUpdate > 500) {
      lastUpdate = millis();
      Serial.printf("-");
    }

  }

  if (!client.available() == 0) {
    Serial.println("Client disconnected before body parsing");
  }

  return result;
}

int EspaperParser::getAndDrawScreen(String requestPath) {

  String url = this->baseUrl + requestPath;
  int httpCode = downloadResource(this->dissectUrl(url), "/screen", 0);
  if (httpCode < 0 || httpCode != 200) {
    gfx->init();
    gfx->fillBuffer(1);
    gfx->setColor(0);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_16);
    gfx->drawString(this->screenWidth / 2, 20, "Error connecting to the server\nHTTP CODE: " + String(httpCode));
    gfx->commit();
    gfx->freeBuffer();
    return false;
  } else {
    gfx->init();
    gfx->fillBuffer(1);
    gfx->setColor(0);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_16);
    gfx->drawPalettedBitmapFromFile(0, 0, "/screen");
    gfx->commit();
    gfx->freeBuffer();
  }
}

EspaperParser::Url EspaperParser::dissectUrl(String url) {
  // Code from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.cpp
  // check for : (http: or https:
  int index = url.indexOf(':');
  if (index < 0) {
    Serial.print("[HTTP-Client][begin] failed to parse protocol\n");
  }

  String _protocol = url.substring(0, index);
  url.remove(0, (index + 3)); // remove http:// or https://

  index = url.indexOf('/');
  String host = url.substring(0, index);
  String _host;
  int _port = 0;
  url.remove(0, index); // remove host part

  // get Authorization
  index = host.indexOf('@');
  if (index >= 0) {
    // auth info
    String auth = host.substring(0, index);
    host.remove(0, index + 1); // remove auth part including @
  }

  // get port
  index = host.indexOf(':');
  if (index >= 0) {
    _host = host.substring(0, index); // hostname
    host.remove(0, (index + 1)); // remove hostname + :
    _port = host.toInt(); // get port
  } else {
    _host = host;
    _port = _protocol == "http" ? 80 : 443;
  }
  String _path = url;

  Url result;

  result.protocol = _protocol;
  result.host = _host;
  result.port = _port;
  result.path = _path;

  return result;
}

WiFiClient EspaperParser::createWifiClient(String protocol) {
  // it would be correct to do this via
  // #ifdef DEV_ENV
  // as it's a compile-time configuration rather than a runtime configuration but including 
  // #include "settings.h" leads to odd compile errors
  WiFiClient client;
  if (String("http").equals(protocol)) {
    Serial.println("Using non-secure WiFi client");
  } else {
    Serial.println("Using secure WiFi client");
    BearSSL::WiFiClientSecure c;
    Serial.println("[HTTP] configuring server root cert in client");
    BearSSLX509List cert(this->rootCert);
    c.setTrustAnchors(&cert);
    client = c;
  }
  return client;
}

int EspaperParser::downloadResource(Url url, String fileName, long expires) {
  Serial.printf("Protocol: %s\n Host: %s\n Port: %d\n URL: %s\n FileName: %s\n Expires: %d\n", url.protocol.c_str(), url.host.c_str(), url.port, url.path.c_str(), fileName.c_str(), expires);

  WiFiClient client = this->createWifiClient(url.protocol);

  FSInfo fs_info;
  SPIFFS.info(fs_info);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  client.connect(url.host, url.port);
  if (!client.connected()) {
    Serial.println("*** Can't connect. ***\n-------");
    return -2;
  }

  String EOL = "\r\n";

  String request = "GET " + url.path + " HTTP/1.1\r\n" +
                   "Host: " + url.host + "\r\n" +
                   "User-Agent: ESPaperClient/1.0\r\n" +
                   "X-ESPAPER-SECRET: " + this->deviceSecret + EOL +
                   "X-ESPAPER-CLIENT-VERSION: " + this->clientVersion + EOL +
                   "X-ESPAPER-BATTERY: " + String(analogRead(A0)) + EOL +
                   "X-ESPAPER-SPIFFS-FREE: " + (fs_info.totalBytes - fs_info.usedBytes) + EOL +
                   "X-ESPAPER-SPIFFS-TOTAL: " + String(fs_info.totalBytes) + EOL +
                   "X-ESPAPER-MILLIS: " + String(millis()) + "\r\n" +
                   "X-ESPAPER-FREE-HEAP: " + String(ESP.getFreeHeap()) + EOL +
                   "X-ESPAPER-WIFI-RSSI: " + String(WiFi.RSSI()) + EOL +
                   "Connection: close\r\n\r\n";

  Serial.println("Sending request: " + request);

  client.print(request);

  long lastUpdate = millis();

  int httpCode = 0;
  while (client.available() || client.connected()) {
    String line = client.readStringUntil('\n');
    Serial.println(line);

    if (line.startsWith("HTTP/1.")) {
      httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
      Serial.printf("HTTP Code: %d\n", httpCode);
    }
    if (line == "\r" || line == "\r\n") {
      Serial.println("headers received");
      break;
    }
    if (millis() - lastUpdate > 500) {
      lastUpdate = millis();
      Serial.printf("-");
    }

  }

  if (!client.available() == 0) {
    Serial.println("Client disconnected before body parsing");
  }
  Serial.println("Processing body");

  long downloadedBytes = 0;
  if (httpCode > 0) {

    // file found at server
    if (httpCode == 200) {
      File file = SPIFFS.open(fileName, "w+");
      if (!file) {
        Serial.println("Creating file failed: " + fileName);
      }
      // get lenght of document (is -1 when Server sends no Content-Length header)

      // Serial.printf("Payload size: %d\n", len);
      // create buffer for read
      uint8_t buff[128] = { 0 };

      Serial.println("Starting resource download");
      // read all data from server

      while (client.available() || client.connected()) {
        // get available data size
        size_t size = client.available();

        if (size > 0) {
          // read up to 1024 byte
          int c = client.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          downloadedBytes += c;

          file.write(buff, c);
          Serial.printf("%d\n", (int)downloadedBytes);
        }
        if (millis() - lastUpdate > 500) {
          lastUpdate = millis();
          Serial.printf("Bytes downloaded: %d\n", downloadedBytes);
        }

      }
      file.close();
      client.stop();
      Serial.printf("Downloaded file %s with size %d", fileName.c_str(), downloadedBytes);
      Serial.println();
      Serial.print("[HTTP] connection closed or file end.\n");
      return httpCode;
    } else {
      client.stop();
      return httpCode;
    }
  } else {
    client.stop();
    return httpCode;
  }

  client.stop();
  return -2;
}
