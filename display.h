#ifndef DISPLAY_H
#define DISPLAY_H

#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include "config.h"

extern Adafruit_SSD1306 oled;

void initOled();
void drawEnvScreen(float temperatureC, float humidityPct, int lightRaw);
void drawDebugScreen(bool sensorOk);
void updateDisplayBrightness(int lightRaw);

#endif