/**
 * @file      TFT_eSPI_Sprite.ino
 * @author    Lewis He (lewishe@outlook.com)
 * @license   MIT
 * @copyright Copyright (c) 2023  Shenzhen Xin Yuan Electronic Technology Co., Ltd
 * @date      2023-06-14
 *
 */

#include "rm67162.h"
#include <TFT_eSPI.h>   //https://github.com/Bodmer/TFT_eSPI
#include "true_color.h"
#include <WiFi.h>
#include <HTTPClient.h>

const char* ssid = "Vodafone-123";  // Replace with your SSID
const char* password = "qwertyqwerty";  // Replace with your Wi-Fi password


#if ARDUINO_USB_CDC_ON_BOOT != 1
#warning "If you need to monitor printed data, be sure to set USB CDC On boot to ENABLE, otherwise you will not see any data in the serial monitor"
#endif

#ifndef BOARD_HAS_PSRAM
#error "Detected that PSRAM is not turned on. Please set PSRAM to OPI PSRAM in ArduinoIDE"
#endif

TFT_eSPI tft = TFT_eSPI();
TFT_eSprite spr = TFT_eSprite(&tft);


#define WIDTH  536
#define HEIGHT 240
unsigned long targetTime = 0;
byte red = 31;
byte green = 0;
byte blue = 0;
byte state = 0;
unsigned int colour = red << 11;

void setup(){
    // Use TFT_eSPI Sprite made by framebuffer , unnecessary calling during use tft.xxxx function
    Serial.begin(115200);
    uint16_t colors[6] = {TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA};
    rm67162_init();
    lcd_setRotation(1);
    spr.createSprite(WIDTH, HEIGHT);
    spr.setSwapBytes(1);
    Serial.println("START");

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(1000);
      Serial.println("Connecting to WiFi...");
    }

        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(colors[1], TFT_BLACK);
        spr.drawString("wifi ok", WIDTH / 2 - 30, 85, 4);
        lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());

    HTTPClient http;
    http.begin("http://api.geqo.it/ingr.bmp");
    int httpCode = http.GET();

    if (httpCode == 200) {

        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(colors[1], TFT_BLACK);
        spr.drawString("url ok", WIDTH / 2 - 30, 85, 4);
        lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());

      WiFiClient *stream = http.getStreamPtr();
      uint8_t bmpHeader[54];
      stream->readBytes(bmpHeader, 54);
      int32_t bmpWidth = *(int32_t *)&bmpHeader[18];
      int32_t bmpHeight = *(int32_t *)&bmpHeader[22];

      // Check if the BMP is uncompressed
      if (bmpHeader[0] != 'B' || bmpHeader[1] != 'M' || *(int32_t *)&bmpHeader[30] != 0) {
          Serial.println("Invalid BMP format");
          spr.fillSprite(TFT_BLACK);
          spr.setTextColor(colors[1], TFT_BLACK);
          spr.drawString("bmp ko", WIDTH / 2 - 30, 85, 4);
          lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
          return;
      }
        spr.fillSprite(TFT_BLACK);
        spr.setTextColor(colors[1], TFT_BLACK);
        spr.drawString("bmp ok", WIDTH / 2 - 30, 85, 4);
        lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());

      spr.fillSprite(TFT_BLACK);
      int padding = (4 - (bmpWidth * 3) % 4) % 4;
      spr.createSprite(bmpWidth, bmpHeight);
      uint8_t *lineBuffer = (uint8_t *)malloc(bmpWidth * 3);
      for (int y = 0; y < bmpHeight; y++) {
          stream->readBytes(lineBuffer, bmpWidth * 3);
          for (int x = 0; x < bmpWidth; x++) {
              uint16_t color = spr.color565(lineBuffer[x * 3 + 2], lineBuffer[x * 3 + 1], lineBuffer[x * 3]);
              spr.drawPixel(x, bmpHeight - y - 1, color);  // Draw in sprite buffer
          }
          stream->readBytes(lineBuffer, padding);  // Skip padding
      }
      free(lineBuffer);
      spr.pushSprite(0, 0);
      lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());

      //lcd_PushColors(0, 0, WIDTH, HEIGHT, (uint16_t *)spr.getPointer());
    } else {
        Serial.println("Error in HTTP request");
    }
    http.end();

        


}

void loop()
{
        Serial.println("Ethernet Reconnected");
    delay(2000);


}

