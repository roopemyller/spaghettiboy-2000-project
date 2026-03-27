#ifndef DISPLAY_H
#define DISPLAY_H


#include <U8g2lib.h>
#include "config.h"

enum AppState : uint8_t {
	UI_SPLASH = 0,
	UI_INFO   = 1,
	UI_GAME   = 2
};

extern U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2;

void initOled();
void renderSplash();
void renderInfo();
void renderGame();
void renderCurrentState(AppState state);
void drawDebugScreen(bool sensorOk);
void updateDisplayBrightness(int lightRaw);

#endif