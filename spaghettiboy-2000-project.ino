#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "display.h"
#include "sensors.h"

enum AppState {
  BOOT,
  SHOW_ENV,
  SHOW_DEBUG
};

AppState state = BOOT;
unsigned long lastReadMs = 0;
unsigned long bootStartMs = 0;

void setup() {
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BUTTON_MODE, INPUT_PULLUP);
  pinMode(PIN_STATUS_LED, OUTPUT);

  Serial.begin(SERIAL_BAUD);
  Wire.begin();

  initSensors();
  initOled();
  bootStartMs = millis();
}

void loop() {
  const unsigned long now = millis();

  updateDisplayBrightness(lightRaw);

  if (state == BOOT) {
    digitalWrite(PIN_STATUS_LED, (now / 200) % 2);
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(SSD1306_WHITE);
    oled.setCursor(0, 0);
    oled.println(F("Booting..."));
    oled.display();

    if (now - bootStartMs > 1200) {
      state = SHOW_ENV;
    }
    return;
  }

  if (digitalRead(PIN_BUTTON_MODE) == LOW) {
    delay(120);
    state = (state == SHOW_ENV) ? SHOW_DEBUG : SHOW_ENV;
  }

  if (now - lastReadMs >= READ_INTERVAL_MS) {
    readSensors();
    lastReadMs = now;
  }

  digitalWrite(PIN_STATUS_LED, HIGH);

  if (state == SHOW_ENV) {
    drawEnvScreen(temperatureC, humidityPct, lightRaw);
  } else {
    drawDebugScreen(sensorOk);
  }
}
