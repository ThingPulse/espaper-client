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
  this->rootCertificate = rootCertificate;
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
  WiFiClient *client = this->createWifiClient(url);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  client->connect(url.host.c_str(), url.port);
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
                   "X-ESPAPER-BATTERY: " + String(Board.getBattery()) + EOL +
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

EspaperParser::ResourceResponse EspaperParser::getAndDrawScreen(String requestPath, String optionalHeaderFields, EspaperParser::HandlerFunction downloadCompletedFunction) {

  String url = this->baseUrl + requestPath;
  EspaperParser::ResourceResponse response = downloadResource(this->dissectUrl(url), "/screen", optionalHeaderFields);
  downloadCompletedFunction();

  if (response.httpCode == HTTP_INTERNAL_CODE_UPGRADE_CLIENT) {
    // In case of update return before the framebuffer is allocated, since it fragments the 
    // memory too much
    Serial.println(F("Update requested"));
    return response;
  }

  gfx->init();
  gfx->fillBuffer(1);


  if (response.httpCode == 200) {
    gfx->drawPalettedBitmapFromFile(0, 0, "/screen");
  } else {
    uint16_t halfWidth = gfx->getWidth() / 2;
    uint16_t maxTextWidth = gfx->getWidth() * 0.85;

    gfx->setColor(0);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_16);
    
    String message = "";
    switch(response.httpCode) {
      case -2:  message = String(F("Connection to the server could not be established. Verify this device has access to the internet."));
                break;
                // TODO: "Starting registration process." is not correct, this parser can't possibly know that...
      case 410: message = String(F("This device is unknown to the server. It might have been deleted. Starting registration process."));
                break;
      default:  message = String(F("Error communicating with the server. HTTP status: ")) + String(response.httpCode);
                break;
    }
    
    gfx->drawStringMaxWidth(halfWidth, 20, maxTextWidth, message);
  }
  Serial.println(F("Writting image to screen"));
  gfx->commit();
  Serial.println(F("De-allocating frame buffer"));
  gfx->freeBuffer();
    
  return response;
}

EspaperParser::Url EspaperParser::dissectUrl(String url) {
  Url result;
  
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


  result.protocol = _protocol;
  result.host = _host;
  result.port = _port;
  result.path = _path;

  return result;
}

WiFiClient* EspaperParser::createWifiClient(Url url) {
  // it would be correct to do this via
  // #ifdef DEV_ENV
  // as it's a compile-time configuration rather than a runtime configuration but including 
  // #include "settings.h" leads to odd compile errors
  if (String("http").equals(url.protocol)) {
    Serial.println("Using non-secure WiFi client");
    return new WiFiClient();
  } else {
    Serial.println("Using secure WiFi client");
    WiFiClientSecure *client = new WiFiClientSecure();
    Serial.println("[HTTP] configuring server root cert in client");
    #if defined(ESP8266) 
      client->setTrustAnchors(new X509List(this->rootCertificate));
      bool mfln = client->probeMaxFragmentLength(url.host, url.port, 1024); 
      Serial.printf("MFLN supported: %s\n", mfln ? "yes" : "no");
      if (mfln) {
        client->setBufferSizes(1024, 1024);
      }
    #elif defined(ESP32)
      client->setCACert(this->rootCertificate);
    #endif
    return client;
  }

}

EspaperParser::ResourceResponse EspaperParser::downloadResource(Url url, String fileName, String optionalHeaderFields) {
  EspaperParser::ResourceResponse response;
  response.httpCode = 0;
  response.sleepSeconds = 0;
  response.sleepUntilEpoch = 0;

  Serial.printf(PSTR("Downloading resource from:\n\tScheme: %s\n\tHost: %s\n\tPort: %d\n\tPath: %s\n"), url.protocol.c_str(), url.host.c_str(), url.port, url.path.c_str());

  WiFiClient *client = this->createWifiClient(url);

  Serial.println("[HTTP] begin...");

  Serial.printf("Connecting to %s:%d\n", url.host.c_str(), url.port);
  Serial.printf("Free mem: %d\n",  ESP.getFreeHeap());
  client->connect(url.host.c_str(), url.port);
  if (!client->connected()) {
    Serial.println("*** Can't connect. ***\n-------");
    delete client;
    response.httpCode = -2;
    return response;
  }

  String EOL = "\r\n";

  // keep the X-ESPAPER headers alphabetically sorted
  String request = "GET " + url.path + " HTTP/1.1\r\n" +
                   "Host: " + url.host + "\r\n" +
                   "User-Agent: ESPaperClient/1.0\r\n" +
                   "X-ESPAPER-BATTERY: " + String(Board.getBattery()) + EOL +
                   "X-ESPAPER-CLIENT-VERSION: " + this->clientVersion + EOL +
                   "X-ESPAPER-FREE-HEAP: " + String(ESP.getFreeHeap()) + EOL +
                   "X-ESPAPER-MILLIS: " + String(millis()) + "\r\n" +
                   // "X-ESPAPER-RESET-REASON: " + rtc_get_reset_reason(0) + EOL +
                   "X-ESPAPER-SECRET: " + this->deviceSecret + EOL +
                   "X-ESPAPER-SPIFFS-FREE: " + String(Board.getFreeSPIFFSBytes()) + EOL +
                   "X-ESPAPER-SPIFFS-TOTAL: " + String(Board.getTotalSPIFFSBytes()) + EOL +
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
      response.httpCode = -2;
      return response;
    }
  }

  long lastUpdate = millis();

  while (client->available() || client->connected()) {
    String line = client->readStringUntil('\n');
    Serial.println(line);
    line.toUpperCase();
    if (line.startsWith("HTTP/1.")) {
      response.httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
    } else if (line.startsWith("X-ESPAPER-COMMAND: UPDATE")) {
      Serial.println("Server requests firmware update");
      response.httpCode = HTTP_INTERNAL_CODE_UPGRADE_CLIENT;
      return response;
    } else if (line.startsWith("X-ESPAPER-SLEEP-SECONDS:")) {
      response.sleepSeconds = line.substring(24).toInt();
    } else if (line.startsWith("X-ESPAPER-SLEEP-UNTIL:")) {
      response.sleepUntilEpoch = line.substring(22).toInt();
    } else if (line == "\r" || line == "\r\n") {
      Serial.println("headers received");
      Serial.printf("Parsed HTTP code: %d\n", response.httpCode);
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
  if (response.httpCode > 0) {

    // file found at server
    if (response.httpCode == 200) {
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
      return response;
    } else {
      client->stop();
      delete client;
      return response;
    }
  } else {
    client->stop();
    delete client;
    return response;
  }

  client->stop();
  delete client;
  response.httpCode = -2;
  return response;
}

// As per https://arduino-esp8266.readthedocs.io/en/latest/ota_updates/readme.html#advanced-updater
void EspaperParser::updateFirmware(String requestPath) {
  String urlPath = this->baseUrl + requestPath;
  Url url = this->dissectUrl(urlPath);
  Serial.printf(PSTR("Updating firmware from:\n\tScheme: %s\n\tHost: %s\n\tPort: %d\n\tPath: %s\n"), url.protocol.c_str(), url.host.c_str(), url.port, url.path.c_str());
  WiFiClient *client = this->createWifiClient(url);

  Serial.printf(PSTR("Free sketch space: %d\n"), ESP.getFreeSketchSpace());
  HTTP_UPDATER.rebootOnUpdate(false);
  t_httpUpdate_return ret = HTTP_UPDATER.update(*client, urlPath);
  
  Serial.print("Status code after firmware update: ");
  Serial.println(ret);
  // As per https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266httpUpdate/src/ESP8266httpUpdate.h#L57 there are currently 3 values supported
  switch (ret) {
    case HTTP_UPDATE_FAILED:
      Serial.printf(PSTR("HTTP_UPDATE_FAILED Error (%d): %s\n"), HTTP_UPDATER.getLastError(), HTTP_UPDATER.getLastErrorString().c_str());
      break;

    case HTTP_UPDATE_NO_UPDATES:
      // no further info to print
      break;

    case HTTP_UPDATE_OK:
      // no further info to print
      break;

    default:
      Serial.printf_P(PSTR("%s is an unexpected return value. Did the library change?\n"), ret);
      break;  
  }
 
  client->stop();
  delete client;
}
