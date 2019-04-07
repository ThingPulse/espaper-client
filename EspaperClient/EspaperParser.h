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

#ifndef ESPAPER_PARSER_H
#define ESPAPER_PARSER_H

#include "Board.h"




#include <WiFiClientSecure.h>

#include <MiniGrafx.h>
#include <Arduino.h>
#include <MiniGrafxFonts.h>
//#include <rom/rtc.h>

#define HTTP_INTERNAL_CODE_UPGRADE_CLIENT -5

class EspaperParser {
  public:
    typedef std::function<void(void)> HandlerFunction;

    typedef struct DeviceIdAndSecret {
      String deviceId;
      String deviceSecret;
    } DeviceIdAndSecret;

    typedef struct ResourceResponse {
      int httpCode;
      uint32_t sleepSeconds;
    } ResourceResponse;
  
    EspaperParser(MiniGrafx *gfx, const char *rootCertificate, String baseUrl, String deviceSecret, String clientVersion);

    void updateFirmware(String url);
    
    DeviceIdAndSecret registerDevice(String requestPath, String jsonData);
  
    ResourceResponse getAndDrawScreen(String requestPath, String optionalHeaderFields, HandlerFunction downloadCompletedFunction);
    
  private:
    MiniGrafx *gfx;
    uint8_t screenWidth;
    String baseUrl;
    String requestPath;
    String deviceSecret;
    String clientVersion;
    const char *rootCertificate;

    typedef struct Url {
      String protocol;
      String host;
      uint16_t port; 
      String path;
    } Url;

    ResourceResponse downloadResource(Url url, String fileName, String optionalHeaderFields);
    
    Url dissectUrl(String url);
    WiFiClient* createWifiClient(Url url);  


};


#endif