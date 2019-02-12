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

EspaperParser::EspaperParser(MiniGrafx *gfx, const char *rootCertificate, String baseUrl, String deviceSecret, String clientVersion) {
  this->gfx = gfx;
  this->certList = new BearSSLX509List(rootCertificate);
  this->baseUrl = baseUrl;
  this->deviceSecret = deviceSecret;
  this->clientVersion = clientVersion;
}

EspaperParser::DeviceIdAndSecret EspaperParser::registerDevice(String requestPath, String jsonData) {
  DeviceIdAndSecret result;
  // set default values signifying an error condition
  result.deviceId = "-1";
  result.deviceSecret = "-2";
  
  Url url = this->dissectUrl(this->baseUrl + requestPath);
  Serial.printf("Free mem: %d",  ESP.getFreeHeap());
  WiFiClient *client = this->createWifiClient(url.protocol);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  client->connect(url.host, url.port);
  if (!client->connected()) {
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

  client->print(request);

  unsigned long timeout = millis();
  while (client->available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println(">>> Client Timeout !");
      client->stop();
      delete client;
      return result;
    }
  }

  long lastUpdate = millis();

  int httpCode = 0;
  while (client->available() || client->connected()) {
    String line = client->readStringUntil('\n');
    int endOfKey = line.indexOf(':');
    String key = "";
    if (endOfKey > 0) {
      key = line.substring(0, endOfKey);
      key.toUpperCase();
    }
    Serial.println(line);

    if (line.startsWith("HTTP/1.")) {
      httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
      Serial.printf("HTTP Code: %d\n", httpCode);
    }
    if (key == "X-ESPAPER-SECRET") {
      this->deviceSecret = line.substring(18, line.indexOf('\r'));
      result.deviceSecret = this->deviceSecret;
    }
    if (key == "X-ESPAPER-DEVICE-ID") {
      result.deviceId = line.substring(21, line.indexOf('\r'));
    }
    if (line == "\r" || line == "\r\n") {
      Serial.println("headers received");
      Serial.println("Parsed values:");
      Serial.printf("- HTTP code: %d\n", httpCode);
      Serial.printf("- Device id: %s\n", result.deviceId.c_str());
      Serial.printf("- Device secret: %s\n", result.deviceSecret.c_str());
      break;
    }
    if (millis() - lastUpdate > 500) {
      lastUpdate = millis();
      Serial.printf("-");
    }

  }

  if (!client->available() == 0) {
    Serial.println("Client disconnected before body parsing");
  }
  delete client;
  return result;
}

int EspaperParser::getAndDrawScreen(String requestPath, String optionalHeaderFields, EspaperParser::HandlerFunction downloadCompletedFunction) {

  String url = this->baseUrl + requestPath;
  int httpCode = downloadResource(this->dissectUrl(url), "/screen", optionalHeaderFields);
  downloadCompletedFunction();

  gfx->init();
  gfx->fillBuffer(1);


  if (httpCode == 200) {
    gfx->drawPalettedBitmapFromFile(0, 0, "/screen");
  } else {
    uint16_t halfWidth = gfx->getWidth() / 2;
    uint16_t maxTextWidth = gfx->getWidth() * 0.85;

    gfx->setColor(0);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_16);
    
    String message = "";
    switch(httpCode) {
      case -2:  message = "Connection to the server could not be established. Verify this device has access to the internet.";
                break;
                // TODO: "Starting registration process." is not correct, this parser can't possibly know that...
      case 410: message = "This device is unknown to the server. It might have been deleted. Starting registration process.";
                break;
      default:  message = "Error communicating with the server. HTTP status: " + String(httpCode);
                break;
    }
    
    gfx->drawStringMaxWidth(halfWidth, 20, maxTextWidth, message);
  }
  
  gfx->commit();
  gfx->freeBuffer();
    
  return httpCode;
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

WiFiClient* EspaperParser::createWifiClient(String protocol) {
  // it would be correct to do this via
  // #ifdef DEV_ENV
  // as it's a compile-time configuration rather than a runtime configuration but including 
  // #include "settings.h" leads to odd compile errors
  if (String("http").equals(protocol)) {
    Serial.println("Using non-secure WiFi client");
    return new WiFiClient();
  } else {
    Serial.println("Using secure WiFi client");
    BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure();
    Serial.println("[HTTP] configuring server root cert in client");
    client->setTrustAnchors(this->certList);
    return client;
  }

}

int EspaperParser::downloadResource(Url url, String fileName, String optionalHeaderFields) {
  Serial.printf("Protocol: %s\n Host: %s\n Port: %d\n URL: %s\n FileName: %s\n", url.protocol.c_str(), url.host.c_str(), url.port, url.path.c_str(), fileName.c_str());

  WiFiClient *client = this->createWifiClient(url.protocol);

  FSInfo fs_info;
  SPIFFS.info(fs_info);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  Serial.printf("Free mem: %d\n",  ESP.getFreeHeap());
  client->connect(url.host, url.port);
  if (!client->connected()) {
    Serial.println("*** Can't connect. ***\n-------");
    delete client;
    return -2;
  }

  String EOL = "\r\n";

  // keep the X-ESPAPER headers alphabetically sorted
  String request = "GET " + url.path + " HTTP/1.1\r\n" +
                   "Host: " + url.host + "\r\n" +
                   "User-Agent: ESPaperClient/1.0\r\n" +
                   "X-ESPAPER-BATTERY: " + String(analogRead(A0)) + EOL +
                   "X-ESPAPER-CLIENT-VERSION: " + this->clientVersion + EOL +
                   "X-ESPAPER-FREE-HEAP: " + String(ESP.getFreeHeap()) + EOL +
                   "X-ESPAPER-MILLIS: " + String(millis()) + "\r\n" +
                   "X-ESPAPER-RESET-REASON: " + ESP.getResetReason() + EOL +
                   "X-ESPAPER-SECRET: " + this->deviceSecret + EOL +
                   "X-ESPAPER-SPIFFS-FREE: " + (fs_info.totalBytes - fs_info.usedBytes) + EOL +
                   "X-ESPAPER-SPIFFS-TOTAL: " + String(fs_info.totalBytes) + EOL +
                   "X-ESPAPER-WIFI-RSSI: " + String(WiFi.RSSI()) + EOL +
                   optionalHeaderFields +
                   "Connection: close\r\n\r\n";

  Serial.println("Sending request: " + request);

  client->print(request);

  unsigned long timeout = millis();
  while (client->available() == 0) {
    if (millis() - timeout > 10000) {
      Serial.println(">>> Client Timeout !");
      client->stop();
      delete client;
      return -2;
    }
  }

  long lastUpdate = millis();

  int httpCode = 0;
  while (client->available() || client->connected()) {
    String line = client->readStringUntil('\n');
    Serial.println(line);

    if (line.startsWith("HTTP/1.")) {
      httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
    }
    if (line == "\r" || line == "\r\n") {
      Serial.println("headers received");
      Serial.printf("Parsed HTTP code: %d\n", httpCode);
      break;
    }
    if (millis() - lastUpdate > 500) {
      lastUpdate = millis();
      Serial.printf("-");
    }

  }

  if (!client->available() == 0) {
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

      while (client->available() || client->connected()) {
        // get available data size
        size_t size = client->available();

        if (size > 0) {
          // read up to 1024 byte
          int c = client->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          downloadedBytes += c;

          file.write(buff, c);
          Serial.print("#");
        }
        if (millis() - lastUpdate > 500) {
          lastUpdate = millis();
          Serial.println();
        }

      }
      file.close();
      client->stop();
      delete client;
      Serial.printf("Downloaded file %s with size %d", fileName.c_str(), downloadedBytes);
      Serial.println();
      Serial.print("[HTTP] connection closed or file end.\n");
      return httpCode;
    } else {
      client->stop();
      delete client;
      return httpCode;
    }
  } else {
    client->stop();
    delete client;
    return httpCode;
  }

  client->stop();
  delete client;
  return -2;
}
