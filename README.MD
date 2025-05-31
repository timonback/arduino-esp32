# Playing around with Ideaspark ESP32

> This repo serves as a demo on how to get started, since I found only limited documentation online.
> I do not plan to maintain this and code quality is okayish.

Use the Arduino IDE and install the esp32 boards. Also install the following libraries:
- NTPClient
- Adafruit GFX
- Adafruit ST7789
- Async TCP
- ESP Async WebServer

The program can be compiled as is and upload to the ESP via USB.
Remember to copy and set up [secrets.h](./secrets.h) based on [secrets.h.env](./secrets.h.env).

> NOTE: Although the Ideaspark ESP32 has a USB-C connector, it must be attached to a non USB-C port (classical USB-Type A)

When running, the ESP will show an IP-Address.
Type the IP-Address in the browser to controll the esp via the web interface.
