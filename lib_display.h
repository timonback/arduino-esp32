#ifndef DISPLAY_H
#define DISPLAY_H

#include "lib_fs.h"

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7789.h> // Hardware-specific library for ST7789
#include <SPI.h>             // Arduino SPI library

#define TFT_CS 15  // define chip select pin
#define TFT_DC 2   // define data/command pin
#define TFT_RST 4  // define reset pin, or set to -1 and connect to Arduino RESET pin
#define LCD_BLK 32 // define the backlight pin
#define TFT_WIDTH 320
#define TFT_HEIGHT 170
#define TFT_SEGMENTS 5
#define TFT_PIXELS TFT_WIDTH *TFT_HEIGHT
#define TFT_DRAW_SECTION TFT_PIXELS / TFT_SEGMENTS
Adafruit_ST7789 tft = Adafruit_ST7789(TFT_CS, TFT_DC, TFT_RST);
uint16_t tft_buffer[TFT_DRAW_SECTION];

const char *files[] = {
    // "/red.raw",
    //  "/green.raw",
    // "/blue.raw",
    "/test.raw",
    "/moveit.raw"};

void display_setup() {
  pinMode(LCD_BLK, OUTPUT);
  analogWrite(LCD_BLK, 0x00FF); // Set backlight to maximum brightness

  tft.init(TFT_HEIGHT, TFT_WIDTH, SPI_MODE2);
  tft.setRotation(3);
}

void display_brightness_set(uint8_t brightness)
{
  // Set the backlight brightness
  analogWrite(LCD_BLK, brightness);
}

void display_brightness_fadeIn(uint8_t brightness, int delay_ms)
{
  uint8_t current_brightness= analogRead(LCD_BLK);

  uint8_t delay_per_step = delay_ms / (brightness - current_brightness);
  for (int i = current_brightness; i < brightness; i++)
  {
    analogWrite(LCD_BLK, i);
    delay(delay_per_step);
  }
}
void display_brightness_fadeOut(uint8_t brightness, int delay_ms)
{
  uint8_t current_brightness = analogRead(LCD_BLK);

  uint8_t delay_per_step = delay_ms / (current_brightness - brightness);
  for (int i = current_brightness; i > brightness; i--)
  {
    analogWrite(LCD_BLK, i);
    delay(delay_per_step);
  }
}

#define DISPLAY_STEP_MS 50
void delay_display(int ms)
{
  tft.drawFastHLine(0, TFT_HEIGHT - 1, TFT_WIDTH, ST77XX_BLUE);
  for (int i = 0; i < ms; i+= DISPLAY_STEP_MS)
  {
    tft.drawFastHLine(0, TFT_HEIGHT - 1, TFT_WIDTH * i / ms, ST77XX_RED);
    delay(DISPLAY_STEP_MS);
  }
}

void display_picture(String path)
{
  if (path.length() > 0 && SPIFFS.exists(path))
  {
    for (uint16_t i = 0; i < TFT_SEGMENTS; i++)
    {
      readRGB565File(SPIFFS, path.c_str(), tft_buffer, TFT_DRAW_SECTION, i * TFT_WIDTH * (TFT_HEIGHT / TFT_SEGMENTS));
      tft.drawRGBBitmap(0, i * (TFT_HEIGHT / TFT_SEGMENTS), tft_buffer, TFT_WIDTH, TFT_HEIGHT / TFT_SEGMENTS);
    }
  }
  else
  {
    tft.fillScreen(ST77XX_BLACK);
  }
}

void loop_display_pictures()
{
  for (int file_index = 0; file_index < sizeof(files) / sizeof(files[0]); file_index++)
  {
    display_picture(String(files[file_index]));

    tft.setTextColor(ST77XX_WHITE);
    tft.setCursor(0, 0);
    tft.setTextSize(1);
    tft.println(F("Hello Handsome!"));
    tft.setTextSize(2);
    tft.println(F("Time to"));
    tft.setTextSize(3);
    tft.println(F(" Move it, Move it"));

    delay_display(1000);
  }
}

#endif
