/**
 * Upload using Arduino IDE ESP32 Dev module (Ideaspark ESP32-WROOM-32)
 * with baud rate of 460800

 Upload to LittleFS via https://github.com/earlephilhower/arduino-littlefs-upload
 */

#include "global.h"


void setup(void)
{
  Serial.begin(460800);
  while (!Serial) // Wait for Serial to be ready
  {
    delay(10);
  }
  Serial.println();

  Serial.print(F("setup() running on core "));
  Serial.println(xPortGetCoreID());

  display_setup();
  fs_setup();
  wifi_setup();

  Serial.println(F("Initialized"));

  show_debugging_info();
}

void loop()
{
  Serial.println(F("loop"));

#ifdef FEATURE_WIFI
  timeClient.update();
#endif

  vm.run();

  String path;
  readFileToString(SPIFFS, "/background", path);
  display_picture(path);

  delay_display(10 * 1000);
}

void show_debugging_info()
{
  for (int i = 0; i < 5; i++)
  {
#ifdef FEATURE_WIFI
    timeClient.update();

    Serial.print(F("Wlan connection: "));
    Serial.println(WiFi.status());
    Serial.println(timeClient.getFormattedTime());

    tft.fillScreen(ST77XX_GREEN);
    tft.setTextSize(2);
    tft.setTextColor(ST77XX_BLUE);

    tft.setCursor(20, 0);
    tft.print(F("IP: "));
    tft.println(WiFi.localIP());

    tft.setCursor(20, tft.getCursorY());
    tft.print(F("Now: "));
    tft.println(timeClient.getFormattedTime());
#endif
    delay(1000);
  }
}