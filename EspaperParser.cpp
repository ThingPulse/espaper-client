#include "EspaperParser.h"

#define USE_SERIAL  Serial

EspaperParser::EspaperParser(MiniGrafx *gfx) {
  this->gfx = gfx;
}

void EspaperParser::setRootCertificate(const unsigned char* rootCertificate, uint16_t rootCertificateLength) {
  this->rootCert = rootCertificate;
  this->rootCertLen = rootCertificateLength;
}

int EspaperParser::updateScreen(String baseUrl, String requestPath, String deviceSecret, String clientVersion) {
  this->baseUrl = baseUrl;
  this->requestPath = requestPath;
  this->deviceSecret = deviceSecret;
  this->clientVersion = clientVersion;

  String url = baseUrl + requestPath;
  int httpCode = downloadResource(url, "request.json", 0);
  if (httpCode < 0 || httpCode != 200) {
    gfx->fillBuffer(1);
    gfx->setColor(0);
    gfx->setTextAlignment(TEXT_ALIGN_CENTER);
    gfx->setFont(ArialMT_Plain_16);
    gfx->drawString(296 / 2, 20, "Error connecting to the server\nHTTP CODE: " + String(httpCode));
    gfx->commit();
    return false;
  }

  File request = SPIFFS.open("request.json", "r");
  if (!request) {
    Serial.println("file open failed");
  }

  JsonStreamingParser parser;
  parser.setListener(this);
  isObjectDrawingCommand = false;
  for (int i = 0; i < request.size(); i++) {
    parser.parse(request.read());
  }
  request.close();
  return true;
}


void EspaperParser::whitespace(char c) {
  //Serial.println("whitespace");
}

void EspaperParser::startDocument() {
  //Serial.println("start document");
}

void EspaperParser::key(String key) {
  //Serial.println("key: " + key);
  currentKey = key;
}

void EspaperParser::value(String value) {
  //Serial.println("value: " + value);
  if (currentKey == "command") {
    //Serial.println(value);
    currentCommand = value;
    isObjectDrawingCommand = true;
  } else if (currentKey == "x1") {
    x1 = value.toInt();
  } else if (currentKey == "y1") {
    y1 = value.toInt();
  } else if (currentKey == "x2") {
    x2 = value.toInt();
  } else if (currentKey == "y2") {
    y2 = value.toInt();
  } else  if (currentKey == "x3") {
    x3 = value.toInt();
  } else  if (currentKey == "y3") {
    y3 = value.toInt();
  } else  if (currentKey == "color") {
    color = value.toInt();
  } else  if (currentKey == "radius") {
    radius = value.toInt();
  } else  if (currentKey == "length") {
    length = value.toInt();
  } else  if (currentKey == "text") {
    text = value;
  } else  if (currentKey == "fontFile") {
    fontFile = value;
  } else  if (currentKey == "fontName") {
    fontName = "";
    isPgmFont = true;
    if (value == "ArialPlain10") {
      fontData = ArialMT_Plain_10;
    } else if (value == "ArialPlain16") {
      fontData = ArialMT_Plain_16;
    } else if (value == "ArialPlain24") {
      fontData = ArialMT_Plain_24;
    } else if (value == "MeteoconsPlain42") {
      fontData = Meteocons_Plain_42;
    } else if (value == "MeteoconsPlain21") {
      fontData = Meteocons_Plain_21;
    } else if (value == "MoonphasesPlain36") {
      fontData = MoonPhases_Regular_36;
    } else {
      isPgmFont = false;
      fontName = value;
    }
  } else  if (currentKey == "fontFamily") {
    fontFamily = value;
  } else  if (currentKey == "fontStyle") {
    fontStyle = value;
  } else  if (currentKey == "fontSize") {
    fontSize = value;
  } else  if (currentKey == "textAlignment") {
    if (value == "LEFT") {
      textAlignment = TEXT_ALIGN_LEFT;
    }
    if (value == "RIGHT") {
      textAlignment = TEXT_ALIGN_RIGHT;
    }
    if (value == "CENTER") {
      textAlignment = TEXT_ALIGN_CENTER;
    }
  } else if (currentKey == "expires") {
    expires = value.toInt();
  } else if (currentKey == "url") {
    url = value;
  } else if (currentKey == "format") {
    format = value;
  } else  if (currentKey == "fileName") {
    fileName = value;
  } else {
    Serial.println("Param not recognized: " + currentKey);
  }


}

void EspaperParser::endArray() {
  //Serial.println("end array. ");
  currentArray = "";
  isObjectDrawingCommand = true;
}

void EspaperParser::endObject() {
  if (currentArray == "resources") {
    isObjectDrawingCommand = false;
    if (format == "mgf") {
      this->resourceFormat = format;
      this->resourceUrl = url;
      USE_SERIAL.printf("Usable font resource detected %s with format %s", url.c_str(), resourceFormat.c_str());
    }
  }
  if (isObjectDrawingCommand) {
    isObjectDrawingCommand = false;
    if (currentCommand == "setPixel") {
      gfx->setPixel(x1, y1);
    } else if (currentCommand == "drawLine") {
      USE_SERIAL.printf("->drawLine(%d, %d, %d, %d)\n", x1, y1, x2, y2);
      gfx->drawLine(x1, y1, x2, y2);

    } else if (currentCommand == "fillRect") {
      USE_SERIAL.printf("->fillRect(%d, %d, %d, %d)\n", x1, y1, x2, y2);
      gfx->fillRect(x1, y1, x2, y2);

    } else if (currentCommand == "drawRect") {
      USE_SERIAL.printf("->drawRect(%d, %d, %d, %d)\n", x1, y1, x2, y2);
      gfx->drawRect(x1, y1, x2, y2);

    } else if (currentCommand == "setColor") {
      USE_SERIAL.printf("->setColor(%d)\n", color);
      gfx->setColor(color);

    } else  if (currentCommand == "drawCircle") {
      USE_SERIAL.printf("->drawCircle(%d, %d, %d)\n", x1, y1, radius);
      gfx->drawCircle(x1, y1, radius);

    } else if (currentCommand == "fillCircle") {
      USE_SERIAL.printf("->fillCircle(%d, %d, %d)\n", x1, y1, radius);
      gfx->fillCircle(x1, y1, radius);

    } else  if (currentCommand == "drawHorizontalLine") {
      USE_SERIAL.printf("->drawHorizontalLine(%d, %d, %d)\n", x1, y1, length);
      gfx->drawHorizontalLine(x1, y1, length);
    } else  if (currentCommand == "drawVerticalLine") {
      USE_SERIAL.printf("->drawVerticalLine(%d, %d, %d)\n", x1, y1, length);
      gfx->drawVerticalLine(x1, y1, length);

    } else  if (currentCommand == "drawString") {
      USE_SERIAL.printf("->drawString(%d, %d, %s)\n", x1, y1, text.c_str());
      gfx->drawString(x1, y1, text);

    } else  if (currentCommand == "drawStringMaxWidth") {
      USE_SERIAL.printf("->drawStringMaxWidth(%d, %d, %d, %s)\n", x1, y1, length, text.c_str());
      gfx->drawStringMaxWidth(x1, y1, length, text);

    } else   if (currentCommand == "setFont") {
      USE_SERIAL.printf("->setFont(xx)\n");
      if (resourceFormat == "mgf" && resourceUrl != "") {
        USE_SERIAL.printf("->setFont(%s, %s, %s, %s)\n", resourceUrl.c_str(), fontFamily.c_str(), fontStyle.c_str(), fontSize.c_str());
        setFont(baseUrl + resourceUrl, fontFamily, fontStyle, fontSize);
      } else {
        USE_SERIAL.printf("->setFont skipped. No resource detected.\n");
      }

    } else   if (currentCommand == "setFontFile") {
      USE_SERIAL.printf("->setFontFile(/cache/%s)\n", fontName.c_str());
      gfx->setFontFile("/cache/" + fontName);

    } else  if (currentCommand == "fillTriangle") {
      USE_SERIAL.printf("->fillTriangle(%d, %d, %d, %d, %d, %d)\n", x1, y1, x2, y2, x3, y3);
      gfx->fillTriangle(x1, y1, x2, y2, x3, y3);

    } else  if (currentCommand == "drawTriangle") {
      USE_SERIAL.printf("->drawTriangle(%d, %d, %d, %d, %d, %d)\n", x1, y1, x2, y2, x3, y3);
      gfx->drawTriangle(x1, y1, x2, y2, x3, y3);

    } else   if (currentCommand == "setTextAlignment") {
      USE_SERIAL.printf("->setTextAlignment(xx)\n");
      gfx->setTextAlignment(textAlignment);

    } else  if (currentCommand == "commit") {
      #ifndef DEV_ENV
        USE_SERIAL.println("Turning off WiFi");
        WiFi.forceSleepBegin();
      #endif
      USE_SERIAL.printf("->commit()\n");
      long startTime = millis();
      gfx->commit();
      USE_SERIAL.printf("Time for commit: %d\n", (millis() - startTime));

    } else if (currentCommand == "clear") {
      USE_SERIAL.printf("->clear()\n");
      gfx->clear();

    } else  if (currentCommand == "fillBuffer") {
      gfx->fillBuffer(color);

    } else  if (currentCommand == "downloadFile") {
      downloadResource(this->url, "/cache/" + this->fileName, this->expires);

    } else  if (currentCommand == "drawImage") {
      long beforeImageDownload = millis();
      downloadResource(this->baseUrl + this->url, "/cache/image", 0);
      Serial.printf("Time for image download: %d\n", (millis() - beforeImageDownload));
      gfx->drawPalettedBitmapFromFile(x1, y1, "/cache/image");
      

    } else  if (currentCommand == "drawBmpFromFile") {
      gfx->drawBmpFromFile("/cache/" + fileName, x1, y1);
    } else if (currentCommand == "flushCache") {
      boolean isCacheFlushed = SPIFFS.remove("/cache");
      if (isCacheFlushed) {
        Serial.println("Cache flushed");
      }
      Serial.print("Flushing Cache");
      Dir dir = SPIFFS.openDir("/cache");
      while (dir.next()) {
        String file = dir.fileName();
        Serial.println(file);
        boolean isFileFlushed = SPIFFS.remove(file);
        if (isFileFlushed) {
          Serial.println("Cache flushed: " + file);
        } else {
          Serial.println("Failed to flush cache: " + file);
        }
      }


    } else if (currentCommand == "formatSpiffs") {
      Serial.println("Formatting SPIFFS...");
      SPIFFS.format();
      Serial.println("Formatted SPIFFS...");
    } else {
      Serial.println("No handler for command found: " + currentCommand);
    }
    currentCommand = "";
  }
}

boolean EspaperParser::setFont(String url, String fontFamily, String fontStyle, String fontSize) {
  String fileName = fontFamily + fontSize + fontStyle +  ".bin";
  downloadResource(url, "/cache/" + fileName, 1);
  gfx->setFontFile("/cache/" + fileName);
}

int EspaperParser::downloadResource(String url, String fileName, long expires) {

    // Code from https://github.com/esp8266/Arduino/blob/master/libraries/ESP8266HTTPClient/src/ESP8266HTTPClient.cpp
    // check for : (http: or https:
    int index = url.indexOf(':');
    if(index < 0) {
        USE_SERIAL.print("[HTTP-Client][begin] failed to parse protocol\n");
        return false;
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
    if(index >= 0) {
        // auth info
        String auth = host.substring(0, index);
        host.remove(0, index + 1); // remove auth part including @
    }

    // get port
    index = host.indexOf(':');
    if(index >= 0) {
        _host = host.substring(0, index); // hostname
        host.remove(0, (index + 1)); // remove hostname + :
        _port = host.toInt(); // get port
    } else {
        _host = host;
        _port = _protocol == "http" ? 80 : 443;
    }
    String _uri = url;

    return this->downloadResource(_protocol, _host, _port, _uri, fileName, expires);
}


int EspaperParser::downloadResource(String protocol, String host, uint16_t port, String url, String fileName, long expires) {
  USE_SERIAL.printf("Protocol: %s\n Host: %s\n Port: %d\n URL: %s\n FileName: %s\n Expires: %d\n", protocol.c_str(), host.c_str(), port, url.c_str(), fileName.c_str(), expires);
  
  String path = fileName;
  String markerFilename = path + "_";
  USE_SERIAL.println("Creating cache marker " + markerFilename);
  File file = SPIFFS.open(markerFilename, "w+");
  if (!file) {
    USE_SERIAL.println("Creating file failed: " + markerFilename);
  }
  file.close();

  if (expires != 0 && SPIFFS.exists(path)) {
    USE_SERIAL.println("File " + path + " already exists. ignoring");
    return false;
  }
  USE_SERIAL.println("Getting " + url);
  USE_SERIAL.printf("Free Heap: %d", ESP.getFreeHeap());

  FSInfo fs_info;
  SPIFFS.info(fs_info);
  USE_SERIAL.printf("FS after cleanup: %d of %d bytes used", fs_info.usedBytes, fs_info.totalBytes);

  USE_SERIAL.print("[HTTP] begin...\n");

  WiFiClientSecure client;

  if (this->rootCertLen > 0) {
    USE_SERIAL.println("Loading root certificate");
    bool res = client.setCACert_P(this->rootCert, this->rootCertLen);
  
    if (!res) {
      Serial.println("Failed to load root CA certificate!");
      while (true) {
        yield();
      }
    }
  }
  USE_SERIAL.printf("Connecting to %s:%d", host.c_str(), port);
  if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return -1;
  }

  /*if (this->rootCertLen > 0 && client.verifyCertChain(host.c_str())) {
    Serial.println("Server certificate verified");
  } else {
    Serial.println("ERROR: certificate verification failed!");
    return -1;
  }*/

  String EOL = "\r\n";

  String request = "GET " + url + " HTTP/1.1\r\n" +
                   "Host: " + host + "\r\n" +
                   "User-Agent: ESPaperClient/1.0\r\n" +
                   "X-ESPAPER-SECRET: " + deviceSecret + EOL +
                   "X-ESPAPER-CLIENT-VERSION: " + this->clientVersion + EOL +
                   "X-ESPAPER-BATTERY: " + String(analogRead(A0)) + EOL +
                   "X-ESPAPER-SPIFFS-FREE: " + (fs_info.totalBytes - fs_info.usedBytes) + EOL +
                   "X-ESPAPER-SPIFFS-TOTAL: " + String(fs_info.totalBytes) + EOL +
                   "X-ESPAPER-MILLIS: " + String(millis()) + "\r\n" +
                   "X-ESPAPER-FREE-HEAP: " + String(ESP.getFreeHeap()) + EOL +
                   "X-ESPAPER-WIFI-RSSI: " + String(WiFi.RSSI()) + EOL +
                   "Connection: close\r\n\r\n";
                   
  USE_SERIAL.println("Sending request: " +request);
  
  client.print(request);
  int httpCode = 0;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    USE_SERIAL.println(line);
    if (line.startsWith("HTTP/1.")) {
      httpCode = line.substring(9, line.indexOf(' ', 9)).toInt();
      USE_SERIAL.printf("HTTP Code: %d\n", httpCode); 
    }
    if (line == "\r" || line == "\r\n") {
      USE_SERIAL.println("headers received");
      break;
    }
  }

  long downloadedBytes = 0;
  if (httpCode > 0) {


    // file found at server
    if (httpCode == 200) {
      File file = SPIFFS.open(path, "w+");
      if (!file) {
        USE_SERIAL.println("Creating file failed: " + path);
      }
      // get lenght of document (is -1 when Server sends no Content-Length header)

      // USE_SERIAL.printf("Payload size: %d\n", len);
      // create buffer for read
      uint8_t buff[128] = { 0 };


      // read all data from server
      long lastUpdate = millis();
      while (client.connected()) {
        // get available data size
        size_t size = client.available();

        if (size) {
          // read up to 1024 byte
          int c = client.readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          downloadedBytes += c;

          file.write(buff, c);

          /*if (len > 0) {
            len -= c;
          }*/
        }
        if (millis() - lastUpdate > 500) {
          lastUpdate = millis();
          //Serial.printf("Bytes left: %d. Available: %d\n", len, size);
        }
      }
      file.close();
      client.stop();
      USE_SERIAL.printf("Downloaded file %s with size %d", fileName.c_str(), downloadedBytes);
      USE_SERIAL.println();
      USE_SERIAL.print("[HTTP] connection closed or file end.\n");
      return httpCode;

    } else {
      client.stop();
      //USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
      return httpCode;
    }
  } else {
    client.stop();
    //USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    return httpCode;//httpCode;
  }
  
  return -2;

}

void EspaperParser::clearUnusedCacheFiles() {
  FSInfo fs_info;
  SPIFFS.info(fs_info);
  USE_SERIAL.printf("FS before cleanup: %d of %d bytes used", fs_info.usedBytes, fs_info.totalBytes);

  Dir dir = SPIFFS.openDir("/cache");
  while (dir.next()) {
    String fileName = dir.fileName();
    USE_SERIAL.println(fileName);
    if (fileName.lastIndexOf("_") == -1) {
      if (!SPIFFS.exists(fileName + "_")) {
        USE_SERIAL.printf("File %s was not used. Purging from Cache\n", fileName.c_str());
        boolean removed = SPIFFS.remove(fileName);
        if (!removed) {
          USE_SERIAL.printf("Purging %s failed\n", fileName.c_str());
        }
      }

    }
  }
  dir = SPIFFS.openDir("/cache");
  while (dir.next()) {
    String fileName = dir.fileName();
    if (fileName.lastIndexOf("_") != -1) {
      USE_SERIAL.printf("Purging marker %s\n", fileName.c_str());
      boolean removed = SPIFFS.remove(fileName);
      if (!removed) {
        USE_SERIAL.printf("Purging %s failed\n", fileName.c_str());
      }

    }
  }
  USE_SERIAL.println("Current cache content after cleanup: ");
  dir = SPIFFS.openDir("/cache");
  while (dir.next()) {
    String fileName = dir.fileName();
    USE_SERIAL.printf("-> %s\n", fileName.c_str());
  }
  SPIFFS.end();
  SPIFFS.begin();
  SPIFFS.info(fs_info);
  USE_SERIAL.printf("FS after cleanup: %d of %d bytes used", fs_info.usedBytes, fs_info.totalBytes);
}

void EspaperParser::endDocument() {
  //Serial.println("end document. ");
  clearUnusedCacheFiles();
}

void EspaperParser::startArray() {
  currentArray = currentKey;
}

void EspaperParser::startObject() {
  //Serial.println("start object. ");
}

