#ifndef _ESPAPERPARSERH_
#define _ESPAPERPARSERH_

#include <WiFiClientSecure.h>
#include <ESP8266WiFi.h>
#include <MiniGrafx.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <Arduino.h>
#include <MiniGrafxFonts.h>
#include "WeatherFonts.h"

#define MAX_CACHED_FILES 15

class EspaperParser: public JsonListener {

  private:
    MiniGrafx *gfx;
    String currentCommand;
    String currentKey;
    String currentArray;
    boolean isObjectDrawingCommand = false;

    TEXT_ALIGNMENT textAlignment = TEXT_ALIGN_LEFT;
    boolean isPgmFont = true;
    const char *fontData = ArialMT_Plain_10;
    const unsigned char* rootCert;
    uint16_t rootCertLen;
    String baseUrl;
    String requestPath;
    String deviceSecret;
    String clientVersion;
    

    // Command variables
    String url;
    String format;
    String resourceUrl;
    String resourceFormat;
    String fileName;
    String fontFamily;
    String fontSize;
    String fontName;
    String fontStyle;
    long expires;
    int16_t x1, y1, x2, y2, x3, y3;
    uint16_t color;
    uint16_t radius;
    uint16_t length;
    String text;
    String fontFile;


  public:
    EspaperParser(MiniGrafx *gfx);

    void setRootCertificate(const unsigned char* rootCertificate, uint16_t rootCertificateLength);
  
    int updateScreen(String baseUrl, String requestPath, String deviceSecret, String clientVersion);

    int downloadResource(String url, String fileName, long expires);

    int downloadResource(String protocol, String host, uint16_t port, String path, String fileName, long expires);

    boolean setFont(String url, String fontFamily, String fontStyle, String fontSize);

    void clearUnusedCacheFiles();
  
    virtual void whitespace(char c);
  
    virtual void startDocument();

    virtual void key(String key);

    virtual void value(String value);

    virtual void endArray();

    virtual void endObject();

    virtual void endDocument();

    virtual void startArray();

    virtual void startObject();
};
#endif
