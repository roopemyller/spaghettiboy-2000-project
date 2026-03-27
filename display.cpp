
#include "display.h"
#include "sensors.h"
#include "config.h"
#include <stdlib.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void initOled() {
  u8g2.setI2CAddress(OLED_ADDR << 1);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setContrast(180);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 12, "Display init");
  } while (u8g2.nextPage());
}

void renderSplash() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(30, 14, "(o)");
    u8g2.drawStr(12, 30, "SpaghettiBOY");
    u8g2.drawStr(30, 42, "-2000-");
    u8g2.drawStr(2, 62, "<LEFT   RIGHT>");
  } while (u8g2.nextPage());
}

void renderInfo() {
  char line[24];
  char valueBuf[12];

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 10, "INFO");

    if (isnan(temperatureC)) {
      u8g2.drawStr(0, 26, "T: N/A");
    } else {
      dtostrf(temperatureC, 0, 1, valueBuf);
      snprintf(line, sizeof(line), "T: %s C", valueBuf);
      u8g2.drawStr(0, 26, line);
    }

    if (isnan(humidityPct)) {
      u8g2.drawStr(0, 40, "H: N/A");
    } else {
      dtostrf(humidityPct, 0, 1, valueBuf);
      snprintf(line, sizeof(line), "H: %s %%", valueBuf);
      u8g2.drawStr(0, 40, line);
    }

    snprintf(line, sizeof(line), "LDR: %d", lightRaw);
    u8g2.drawStr(0, 54, line);
    u8g2.drawStr(0, 62, "LEFT: Splash");
  } while (u8g2.nextPage());
}

void renderGame() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 14, "GAME SCREEN");
    u8g2.drawStr(0, 34, "Content later");
    u8g2.drawStr(0, 62, "RIGHT: Splash");
  } while (u8g2.nextPage());
}

void renderCurrentState(AppState state) {
  switch (state) {
    case UI_SPLASH: renderSplash(); break;
    case UI_INFO:   renderInfo();   break;
    case UI_GAME:   renderGame();   break;
  }
}

void drawDebugScreen(bool sensorOk) {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    u8g2.drawStr(0, 12, "DEBUG");
    u8g2.drawStr(0, 30, sensorOk ? "Sensor init: OK" : "Sensor init: FAIL");
  } while (u8g2.nextPage());
}

void updateDisplayBrightness(int lightRaw) {
  int targetContrast = map(constrain(lightRaw, 0, 1023), 0, 1023, 5, 255);
  u8g2.setContrast(targetContrast);
}