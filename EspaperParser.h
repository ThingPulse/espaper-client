#ifndef _ESPAPERPARSERH_
#define _ESPAPERPARSERH_

#include <ESP8266HTTPClient.h>
#include <MiniGrafx.h>
#include <JsonListener.h>
#include <JsonStreamingParser.h>
#include <Arduino.h>
#include <MiniGrafxFonts.h>
#include "WeatherFonts.h"



class EspaperParser: public JsonListener {

  private:
    MiniGrafx *gfx;
    String currentCommand;
    String currentKey;
    boolean isObjectDrawingCommand = false;
    int16_t x1, y1, x2, y2, x3, y3;
    uint16_t color;
    uint16_t radius;
    uint16_t length;
    String text;
    String fontFile;
    TEXT_ALIGNMENT textAlignment = TEXT_ALIGN_LEFT;
    boolean isPgmFont = true;
    const char *fontData = ArialMT_Plain_10;
    long expires;
    String url;
    String fileName;
    String fontFamily;
    String fontSize;
    String fontName;
    String fontStyle;


  public:
    EspaperParser(MiniGrafx *gfx);
  
    void updateScreen(String url);

    boolean downloadResource(String url, String fileName, long expires);
  
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
