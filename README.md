# ThingPulse ESPaper client

How about *designing* your [ESPaper](https://thingpulse.com/product-category/espaper-epaper-kits/) content rather than programming it? You can fully customize the content to display. And load content from remote resources (calendars etc.). And much more. All with the help of [https://www.espaper.com](https://www.espaper.com).

No Arduino programming required anymore!

This client here is responsible for a simple & smooth registration process and for the general update cycle:

- connect to WiFi
- pull image from server
- render it
- go back to deep sleep
- repeat

![](https://docs.thingpulse.com/img/products/ThingPulse-ESPaper-plus-kit.jpg)

## Service level promise

<table><tr><td><img src="https://thingpulse.com/assets/ThingPulse-open-source-prime.png" width="150">
</td><td>This is a ThingPulse <em>prime</em> project. See our <a href="https://thingpulse.com/about/open-source-commitment/">open-source commitment declaration</a> for what this means.</td></tr></table>


## Setup

1. Go to [https://www.espaper.com](https://www.espaper.com) and create an account. You can either use email and password for authentication or use one of the social login options.
1. [Install drivers for USB-to-Serial](https://docs.thingpulse.com/how-tos/install-drivers/)
1. [Prepare the Arduino IDE for ESP8266](https://docs.thingpulse.com/how-tos/Arduino-IDE-for-ESP8266/)
	- You need at least ESP8266 Arduino Core 2.5.0
1. Download or clone this repository to your computer. Then open it in the Arduino IDE.
1. Install the [MiniGrafx library](https://github.com/ThingPulse/minigrafx) through the library manager in the Arduino IDE.
1. Define the device type in [`settings.h:31ff`](https://github.com/ThingPulse/espaper-client/blob/master/settings.h#L31). Hints: 'EDP' = ESPaper Display, '29' = 2.9''
1. Compile and upload the sketch to your ESPaper module. Then Restart it.
	- in Tools > Board: * > select "Generic ESP8266 Module"
	- in Tools > Flash Mode select "QIO"
	- in Tools > Flash Size select "2M (512K SPIFFS)"
1. Follow the instructions displayed on screen.
1. Initiate the registration process by restarting the device.
1. Now go back to [https://www.espaper.com](https://www.espaper.com) and complete registration process by adding your device.
1. Design the screen for your device.

## Support

Pre-sales: [https://thingpulse.com/about/contact/](https://thingpulse.com/about/contact/)

Customer support: [https://support.thingpulse.com/](https://support.thingpulse.com/)
