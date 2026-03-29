
#include "display.h"
#include "sensors.h"
#include "config.h"
#include <stdlib.h>

U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

static void drawCenteredText(uint8_t y, const char* text) {
  const int16_t textWidth = (int16_t)u8g2.getStrWidth(text);
  int16_t x = ((int16_t)OLED_WIDTH - textWidth) / 2;
  if (x < 0) {
    x = 0;
  }
  u8g2.drawStr((uint8_t)x, y, text);
}

void initOled() {
  u8g2.setI2CAddress(OLED_ADDR << 1);
  u8g2.begin();
  u8g2.setPowerSave(0);
  u8g2.setContrast(180);
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    drawCenteredText(12, "Display init");
  } while (u8g2.nextPage());
}

void renderSplash() {
  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    drawCenteredText(14, "(o)");
    drawCenteredText(30, "SpaghettiBOY");
    drawCenteredText(42, "-2000-");
    drawCenteredText(62, "<LEFT   RIGHT>");
  } while (u8g2.nextPage());
}

void renderInfo() {
  char line[24];
  char valueBuf[12];

  u8g2.firstPage();
  do {
    u8g2.setFont(u8g2_font_6x10_tf);
    drawCenteredText(10, "INFO");

    if (isnan(temperatureC)) {
      drawCenteredText(26, "T: N/A");
    } else {
      dtostrf(temperatureC, 0, 1, valueBuf);
      snprintf(line, sizeof(line), "T: %s C", valueBuf);
      drawCenteredText(26, line);
    }

    if (isnan(humidityPct)) {
      drawCenteredText(40, "H: N/A");
    } else {
      dtostrf(humidityPct, 0, 1, valueBuf);
      snprintf(line, sizeof(line), "H: %s %%", valueBuf);
      drawCenteredText(40, line);
    }

    snprintf(line, sizeof(line), "LDR: %d", lightRaw);
    drawCenteredText(54, line);
    drawCenteredText(62, "<LEFT   RIGHT>");
  } while (u8g2.nextPage());
}

void renderCurrentState(AppState state) {
  switch (state) {
    case UI_SPLASH: renderSplash(); break;
    case UI_INFO:   renderInfo();   break;
    case UI_GAME:                 break;
    case UI_IDLE:                 break;
  }
}

void updateDisplayBrightness(int lightRaw) {
  int targetContrast = map(constrain(lightRaw, 0, 1023), 0, 1023, 5, 255);
  u8g2.setContrast(targetContrast);
}

void setDisplayPowerSave(bool enabled) {
  u8g2.setPowerSave(enabled ? 1 : 0);
}