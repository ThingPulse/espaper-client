#include "EspaperParser.h"

#define USE_SERIAL  Serial

EspaperParser::EspaperParser(MiniGrafx *gfx) {
  this->gfx = gfx;
}

void EspaperParser::updateScreen(String url) {
        USE_SERIAL.println("Getting " + url);
        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");

        // configure server and url
        http.begin(url);

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {

                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();

                // create buffer for read
                uint8_t buff[128] = { 0 };

                // get tcp stream
                WiFiClient * stream = http.getStreamPtr();
                
                JsonStreamingParser parser;
                parser.setListener(this);
                isObjectDrawingCommand = false;
                // read all data from server
                while(http.connected() && (len > 0 || len == -1)) {
                    // get available data size
                    size_t size = stream->available();

                    if(size) {
                        // read up to 128 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));

                        // write it to Serial
                        //USE_SERIAL.write(buff, c);
                        for (uint8_t i = 0; i < c; i++) {
                          parser.parse(buff[i]);
                        }

                        if(len > 0) {
                            len -= c;
                        }
                    }
                    delay(1);
                }

                USE_SERIAL.println();
                USE_SERIAL.print("[HTTP] connection closed or file end.\n");

            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
            gfx->fillBuffer(1);
            gfx->drawString(20, 20, "Connection failed");
            gfx->commit();
        }

        http.end();
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
  }else if (currentKey == "x1") {
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
  } else  if (currentKey == "url") {
    url = value;
  } else  if (currentKey == "fileName") {
    fileName = value;
  } else {
    Serial.println("Param not recognized: " + currentKey);
  }

 
}

void EspaperParser::endArray() {
  //Serial.println("end array. ");
}

void EspaperParser::endObject() {
  //Serial.println("end object. ");
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
      gfx->setFont(fontData);
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
      USE_SERIAL.printf("->commit()\n");
      gfx->commit();
    } else if (currentCommand == "clear") {
      USE_SERIAL.printf("->clear()\n");
      gfx->clear();
    } else  if (currentCommand == "fillBuffer") {
      gfx->fillBuffer(color);
    } else  if (currentCommand == "downloadFile") {
      downloadResource(this->url, "/cache/" + this->fileName, this->expires);
    } else  if (currentCommand == "drawImage") {
      gfx->drawPalettedBitmapFromFile(x1, y1, "/cache/" + fileName);
    } else  if (currentCommand == "drawBmpFromFile") {
      gfx->drawBmpFromFile("/cache/" + fileName, x1, y1);
    } else if (currentCommand == "registerFont") {
      if (!isPgmFont) {
        downloadResource("http://oleddisplay.squix.ch/rest/binaryFont/" + fontFamily + "/" + fontStyle + "/" + fontSize, "/cache/" + fontName, 0);
      } else {
        Serial.printf("Registering PGM font %s", fontName.c_str());
      }
    } else {
      Serial.println("No handler for command found: " + currentCommand);
    }
  }
}

boolean EspaperParser::downloadResource(String url, String fileName, long expires) {
        String path = fileName;
        if (expires != 0 && SPIFFS.exists(path)) {
          USE_SERIAL.println("File " + path + " already exists. ignoring");
          return false;
        }
        USE_SERIAL.println("Getting " + url);
        HTTPClient http;

        USE_SERIAL.print("[HTTP] begin...\n");

        // configure server and url
        http.begin(url);

        USE_SERIAL.print("[HTTP] GET...\n");
        // start connection and send HTTP header
        int httpCode = http.GET();
        long downloadedBytes = 0;
        if(httpCode > 0) {
            // HTTP header has been send and Server response header has been handled
            USE_SERIAL.printf("[HTTP] GET... code: %d\n", httpCode);

            // file found at server
            if(httpCode == HTTP_CODE_OK) {
                File file = SPIFFS.open(path, "w+");
                if (!file) {
                  USE_SERIAL.println("Creating file failed: " + path);
                }
                // get lenght of document (is -1 when Server sends no Content-Length header)
                int len = http.getSize();
                USE_SERIAL.printf("Payload size: %d\n", len);
                // create buffer for read
                uint8_t buff[1460/2] = { 0 };

                // get tcp stream
                WiFiClient * stream = http.getStreamPtr();
                
                // read all data from server
                long lastUpdate = millis();
                while(http.connected() && (len > 0 || len == -1)) {
                    // get available data size
                    size_t size = stream->available();

                    if(size) {
                        // read up to 1024 byte
                        int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                        downloadedBytes +=c;
                        
                        file.write(buff, c);

                        if(len > 0) {
                            len -= c;
                        }
                    }
                    if (millis() - lastUpdate > 2000) {
                      lastUpdate = millis();
                      Serial.printf("Bytes left: %d. Available: %d\n", len, size);
                    }
                }
                file.close();
                USE_SERIAL.printf("Downloaded file %s with size %d", fileName.c_str(), downloadedBytes);
                USE_SERIAL.println();
                USE_SERIAL.print("[HTTP] connection closed or file end.\n");

            }
        } else {
            USE_SERIAL.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
        }

        http.end();
}

void EspaperParser::endDocument() {
  //Serial.println("end document. ");
}

void EspaperParser::startArray() {
   //Serial.println("start array. ");
}

void EspaperParser::startObject() {
   //Serial.println("start object. ");
}

