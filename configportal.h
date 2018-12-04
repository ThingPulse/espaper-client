/**The MIT License (MIT)
Copyright (c) 2017 by Daniel Eichhorn
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

#include <ESP8266WebServer.h>
#include <MiniGrafx.h>

const char HTTP_HEAD[] PROGMEM            = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/><title>{v}</title>";
const char HTTP_STYLE[] PROGMEM           = "<style>.c{text-align: center;} div,input, select{padding:5px;font-size:1em;} input, select{width:95%;} body{text-align: center;font-family:verdana;} button{border:0;border-radius:0.3rem;background-color:#1fa3ec;color:#fff;line-height:2.4rem;font-size:1.2rem;width:100%;} .q{float: right;width: 64px;text-align: right;} .l{background: url(\"data:image/png;base64,iVBORw0KGgoAAAANSUhEUgAAACAAAAAgCAMAAABEpIrGAAAALVBMVEX///8EBwfBwsLw8PAzNjaCg4NTVVUjJiZDRUUUFxdiZGSho6OSk5Pg4eFydHTCjaf3AAAAZElEQVQ4je2NSw7AIAhEBamKn97/uMXEGBvozkWb9C2Zx4xzWykBhFAeYp9gkLyZE0zIMno9n4g19hmdY39scwqVkOXaxph0ZCXQcqxSpgQpONa59wkRDOL93eAXvimwlbPbwwVAegLS1HGfZAAAAABJRU5ErkJggg==\") no-repeat left center;background-size: 1em;}</style>";
const char HTTP_SCRIPT[] PROGMEM          = "<script>function c(l){document.getElementById('s').value=l.innerText||l.textContent;document.getElementById('p').focus();}</script>";
const char HTTP_HEAD_END[] PROGMEM        = "</head><body><div style='text-align:left;display:inline-block;min-width:260px;'>";
const char HTTP_PORTAL_OPTIONS[] PROGMEM  = "<form action=\"/wifi\" method=\"get\"><button>Configure WiFi</button></form><br/><form action=\"/0wifi\" method=\"get\"><button>Configure WiFi (No Scan)</button></form><br/><form action=\"/i\" method=\"get\"><button>Info</button></form><br/><form action=\"/r\" method=\"post\"><button>Reset</button></form>";
const char HTTP_ITEM[] PROGMEM            = "<div><a href='#p' onclick='c(this)'>{v}</a>&nbsp;<span class='q {i}'>{r}%</span></div>";
const char HTTP_FORM_START[] PROGMEM      = "<form method='post' action='save'><br/>";
const char HTTP_FORM_PARAM[] PROGMEM      = "<label for='{i}'>{p}</label><br/><input id='{i}' name='{n}' maxlength={l}  value='{v}' {c}><br/><br/>";
const char HTTP_FORM_END[] PROGMEM        = "<br/><button type='submit'>save</button></form><br/><form action=\"/reset\" method=\"get\"><button>Restart ESP</button></form>";
const char HTTP_SCAN_LINK[] PROGMEM       = "<br/><div class=\"c\"><a href=\"/wifi\">Scan</a></div>";
const char HTTP_SAVED[] PROGMEM           = "<div>Credentials Saved<br />Trying to connect ESP to network.<br />If it fails reconnect to AP to try again</div>";
const char HTTP_END[] PROGMEM             = "</div></body></html>";
const char HTTP_OPTION_ITEM[] PROGMEM     = "<option value=\"{v}\" {s}>{n}</option>";
const char HTTP_WG_LANGUAGES[] PROGMEM    = "See <a href='https://www.wunderground.com/weather/api/d/docs?d=language-support' target=_blank>Language Codes</a> for explanation.";

ESP8266WebServer server (80);

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
  File f = SPIFFS.open("/espaper.txt", "w+");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  f.print("WIFI_SSID=");
  f.println(WIFI_SSID);
  f.print("WIFI_PASS=");
  f.println(WIFI_PASS);
  f.print("DEVICE_ID=");
  f.println(DEVICE_ID);
  f.print("DEVICE_KEY=");
  f.println(DEVICE_KEY);
  f.close();
  Serial.println("Saved values");
  return true;
}

boolean loadConfig() {
  File f = SPIFFS.open("/espaper.txt", "r");
  if (!f) {
    Serial.println("Failed to open config file");
    return false;
  }
  while(f.available()) {
      //Lets read line by line from the file
      String key = f.readStringUntil('=');
      String value = f.readStringUntil('\r');
      f.read();
      Serial.println(key + " = [" + value + "]");
      Serial.println(key.length());
      if (key == "WIFI_SSID") {
        WIFI_SSID = value;
      }
      if (key == "WIFI_PASS") {
        WIFI_PASS = value;
      }
      if (key == "DEVICE_KEY") {
        DEVICE_KEY = value;
      }
      if (key == "DEVICE_ID") {
        DEVICE_ID = value;
      }

  }
  if (WIFI_SSID == "" || DEVICE_KEY == "" || DEVICE_ID =="") {
    Serial.println("At least one attribute not defined");
    return false;
  }
  f.close();
  Serial.println("Loaded config");
  return true;
}

void handleRoot() {
  String page = FPSTR(HTTP_HEAD);
  page.replace("{v}", "Options");
  page += FPSTR(HTTP_SCRIPT);
  page += FPSTR(HTTP_STYLE);
  //page += _customHeadElement;
  page += FPSTR(HTTP_HEAD_END);
  page += "<h1>ESPaper Configuration</h1>";
  page += "Mac Address: " + WiFi.macAddress() + "<BR/>";
  //page += FPSTR(HTTP_PORTAL_OPTIONS);
  page += FPSTR(HTTP_FORM_START);
  page += getFormField("ssid", "WiFi SSID", "20", WIFI_SSID, "");
  page += getFormField("password", "WiFi Password", "20", WIFI_PASS, "");
  page += getFormField("deviceid", "Device ID", "40", DEVICE_ID, "");
  page += getFormField("devicekey", "Device Secret", "40", DEVICE_KEY, "");
  page += FPSTR(HTTP_FORM_END);
  page += FPSTR(HTTP_END);

  server.sendHeader("Content-Length", String(page.length()));
  server.send(200, "text/html", page);
}

void handleSave() {
  WIFI_SSID = server.arg("ssid");
  WIFI_PASS = server.arg("password");
  DEVICE_ID = server.arg("deviceid");
  DEVICE_KEY = server.arg("devicekey");
  Serial.println(WIFI_SSID);
  Serial.println(WIFI_PASS);
  Serial.println(DEVICE_ID);
  Serial.println(DEVICE_KEY);
  saveConfig();
  handleRoot();
}

void handleNotFound() {
  //digitalWrite ( led, 1 );
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

  server.send ( 404, "text/plain", message );
  //digitalWrite ( led, 0 );
}



void startConfigPortal(MiniGrafx *gfx) {
  Serial.println("Starting config portal...");
  server.on ( "/", handleRoot );
  server.on ( "/save", handleSave);
  server.on ( "/reset", []() {
     saveConfig();
     pinMode(2, INPUT);
     pinMode(0, INPUT);
     ESP.restart();
  } );
  server.onNotFound ( handleNotFound );
  server.begin();
  
  boolean connected = WiFi.status() == WL_CONNECTED;
  Serial.printf("Is WiFi conected: %s\n", connected ? "yes": "no");
  gfx->fillBuffer(1);
  gfx->setColor(0);
  gfx->setTextAlignment(TEXT_ALIGN_CENTER);
  gfx->setFont(ArialMT_Plain_16);
  Serial.printf("Is WiFi conected: %s\n", connected ? "yes": "no");

  if (connected) {
      Serial.println ( "Open browser at http://" + WiFi.localIP() );

      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnected to: " + WiFi.SSID() + "\nOpen browser at\nhttp://" + WiFi.localIP().toString());
      
  } else {
      WiFi.mode(WIFI_AP);
      WiFi.softAP(CONFIG_SSID.c_str());
      IPAddress myIP = WiFi.softAPIP();  
      Serial.println(myIP);
      

      gfx->drawString(296 / 2, 10, "ESPaper Setup Mode\nConnect WiFi to:\n" + CONFIG_SSID + "\nOpen browser at\nhttp://" + myIP.toString());

  }
  Serial.println("Committing.. screen");
  gfx->commit();

  Serial.println ( "HTTP server started" );

  while(true) {
    server.handleClient();
    yield();
  }
}


