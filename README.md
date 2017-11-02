# espaper-client

This code parses and dispalys espaper-json descriptions and displays the results on an ESPaper module. ESPaper module can be ordered here:

 * <https://blog.squix.org/product/2-9-espaper-lite-kit>
 * <https://blog.squix.org/product/2-9-espaper-plus-kit>

 You are supporting the creator of this library.

## Background

In order to speed up development of new content for the ESPaper module I decided to follow a similar architecture as we know and love from HTML fetched from a server and displayed in a browser. However, it would be quite a burden for an ESP8266 to parse HTML. So I decided to create a very simple DSL to describe what should be drawn on the display. The available commands follow very much the methods available in the MiniGrafx library: <https://github.com/squix78/minigrafx>

Drawing a line for instance looks like this:
```
{
  "command": "drawLine",
  "params": {
    "x1": "0",
    "y1": "11",
    "x2": "296",
    "y2": "11"
  }
}
```
which draws a line from (0,11) to (296, 11). Just like the MiniGrafx library the parser is has a state. Colors and fonts are set by a command and used by all following commands until the state is changed. This means that the JSON object is parsed strictly sequential from beginning to end. ESP8266 have only limited memory, so every command and all parameters are parsed. When a command object in JSON finishes the command will be executed. This allows parsing of very big JSON descriptions, since the all information is always directly applied to the MiniGrafx frame buffer.

## espaper-client.ino

This sketch is meant for fast development and not for "production". First the espaper connects to the internet and downloads the linked JSON file from the server. Then it enters the main loop until the FLASH button is pressed. This allows for very quick updates but since the ESP never goes to sleep you shouldn't use it with an ESPaper running from batteries.


## Setup

1) Download install this repository to your local disk.
1) Follow instructions of this repository <https://github.com/squix78/espaper-server-php>
1) Install the following libraries json-streaming-parser and MiniGrafx (both from D. Eichhorn) through the library manager of the Arduino IDE
1) Set your WiFi credentials
1) Point the URL in the INO to your server instance.
1) Compile and upload the sketch to your ESPaper

## Know issues

 * Currently fonts have to be part of the firmware. There are already methods for downloading and using fonts from the file system but there are bugs in the SDK which cause problems with bigger font files.
 * Downloading and usage of images also doesn't work well yet.
