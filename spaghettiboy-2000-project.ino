#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "display.h"
#include "sensors.h"

AppState currentState = UI_SPLASH;
unsigned long lastReadMs = 0;
unsigned long bootStartMs = 0;

bool prevLeftPressed  = false;
bool prevRightPressed = false;

void setState(AppState next) {
  if (next != currentState) {
    currentState = next;
    renderCurrentState(currentState);
  }
}

// -----------------------------
// Input + navigation
// -----------------------------
bool isPressed(uint8_t pin) {
  // Buttons expected with internal pull-up: pressed = LOW.
  return digitalRead(pin) == LOW;
}

void handleNavigation() {
  const bool leftPressed  = isPressed(PIN_BTN_LEFT);
  const bool rightPressed = isPressed(PIN_BTN_RIGHT);

  const bool leftEdge  = leftPressed  && !prevLeftPressed;
  const bool rightEdge = rightPressed && !prevRightPressed;

  prevLeftPressed  = leftPressed;
  prevRightPressed = rightPressed;

  // Minimal debounce cost; keeps code tiny.
  delay(8);

  switch (currentState) {
    case UI_SPLASH:
      if (leftEdge) {
        setState(UI_INFO);
      }
      if (rightEdge) {
        setState(UI_GAME);
      }
      break;

    case UI_INFO:
      if (leftEdge) {
        setState(UI_SPLASH);
      }
      break;

    case UI_GAME:
      if (rightEdge) {
        setState(UI_SPLASH);
      }
      break;
  }
}

void setup() {
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_STATUS_LED, OUTPUT);

  Serial.begin(SERIAL_BAUD);
  Wire.begin();

  initSensors();
  readSensors();
  initOled();
  bootStartMs = millis();
  renderCurrentState(currentState);
}

void loop() {
  const unsigned long now = millis();

  handleNavigation();

  if (now - lastReadMs >= READ_INTERVAL_MS) {
    readSensors();
    lastReadMs = now;
    if (currentState == UI_INFO) {
      renderInfo();
    }
  }

  updateDisplayBrightness(lightRaw);


  /*
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

  if (now - lastReadMs >= READ_INTERVAL_MS) {
    readSensors();
    lastReadMs = now;
  }

  digitalWrite(PIN_STATUS_LED, HIGH);

  if (state == SHOW_ENV) {
    drawEnvScreen(temperatureC, humidityPct, lightRaw);
  }
  */
}
