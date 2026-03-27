
#include "display.h"

Adafruit_SSD1306 oled(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

void initOled() {
    oled.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
    oled.clearDisplay();
    oled.display();
}


void drawEnvScreen(float temperatureC, float humidityPct, int lightRaw) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(F("SpaghettiBoy"));
  oled.println(F("ENV"));

  oled.print(F("T: "));
  if (isnan(temperatureC)) {
    oled.println(F("N/A"));
  } else {
    oled.print(temperatureC, 1);
    oled.println(F(" C"));
  }

  oled.print(F("H: "));
  if (isnan(humidityPct)) {
    oled.println(F("N/A"));
  } else {
    oled.print(humidityPct, 1);
    oled.println(F(" %"));
  }

  oled.print(F("LDR: "));
  oled.println(lightRaw);
  oled.display();
}

void drawDebugScreen(bool sensorOk) {
  oled.clearDisplay();
  oled.setTextSize(1);
  oled.setTextColor(SSD1306_WHITE);
  oled.setCursor(0, 0);
  oled.println(F("DEBUG"));
  oled.print(F("Sensor init: "));
  oled.println(sensorOk ? F("OK") : F("FAIL"));
  oled.print(F("Button: "));
  oled.println(digitalRead(PIN_BUTTON_MODE) == LOW ? F("DOWN") : F("UP"));
  oled.display();
}

void updateDisplayBrightness(int lightRaw) {
  int targetContrast = map(lightRaw, 0, 1024, 5, 255);
  oled.ssd1306_command(SSD1306_SETCONTRAST);
  oled.ssd1306_command(targetContrast);

  oled.ssd1306_command(0xD9); // Pre-charge period
  if (targetContrast < 50) {
    oled.ssd1306_command(0x22); // Erittäin himmeä tila
  } else {
    oled.ssd1306_command(0xF1); // Normaali kirkas tila
  }
}