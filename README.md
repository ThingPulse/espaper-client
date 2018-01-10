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

This sketch is meant for connecting to the espaper web application. After creating an account you can setup a application (defining the source of data and the content) and a device. Creating a device will also give you access credentials you need to copy from the web application to your ESPaper device. When you start the firmware for the first time and the softwaare cannot fine a certain config file it will automatically start in config mode. In this mode it will open up a WiFi access point and start a web server. Instructions will also be displayed on the ESPaper display.
There you'll have to enter WiFi credentials as well as the UUID and the password the ESPaper app has created for you. Restart the ESPaper and if everything went well it will fetch data from the espaper server and display it on your screen.


## Setup

1) Go to https://www.espaper.com and create an account. You can either use email and password for this or use one of the social login options (Twitter, Facebook or Google+)
1) After login into espaper.com add an application on the home screen. Easiest to start with is the Message App.
1) Now add a device on the home screen. After pressing the "Save" button you will be presented with UUID and password which will identify your ESPaper to the web application. Write them down somewhere or keep this screen open.
1) Download install this repository to your local disk.
1) Install the following libraries json-streaming-parser and MiniGrafx (both from D. Eichhorn) through the library manager of the Arduino IDE
1) Compile and upload the sketch to your ESPaper module
1) If everything went well the device will show you instructions on the display. You'll probably have to connect your computer or smartphone to the "ESPaperConfig" access point. 
1) Then open the browser at http://192.168.4.1 or whatever IP is displayed on the ESPaper and enter WiFi credentials as well as the UUID and password you received above. Save the values
1) Restart the ESPaper either with the restart button on the form or with the hardware button. (The restart button on the form might not work the first time after flashing a new firmware)
1) Now go back to https://www.espaper.com and modify the application you earlier created, e.g. to display weather information from wunderground

