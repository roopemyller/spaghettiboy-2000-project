#include <Arduino.h>
#include <Wire.h>

#include "config.h"
#include "display.h"
#include "sensors.h"

void initSnakeGame();
void snakeGameOnEnter();
void snakeGameTick(uint32_t nowMs);
bool snakeGameExitRequested();
void clearSnakeGameExitRequest();
bool snakeGameAllowUiNavigation();
void snakeGameSetPowerSave(bool enabled);

AppState currentState = UI_SPLASH;
AppState lastActiveState = UI_SPLASH;
unsigned long lastReadMs = 0;
unsigned long bootStartMs = 0;

bool prevLeftPressed  = false;
bool prevRightPressed = false;

void setState(AppState next) {
  if (next != currentState) {
    const AppState previous = currentState;
    if (previous != UI_IDLE) {
      lastActiveState = previous;
    }
    currentState = next;
    if (currentState == UI_GAME && previous != UI_IDLE) {
      snakeGameOnEnter();
    }
    renderCurrentState(currentState);
  }
}

bool isIdleSwitchActive() {
  // INPUT_PULLUP with switch to GND: closed means IDLE requested.
  return digitalRead(PIN_KILL_SWITCH) == LOW;
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
      } else if (rightEdge) {
        setState(UI_GAME);
      }
      break;

    case UI_INFO:
      if (leftEdge) {
        setState(UI_SPLASH);
      } else if (rightEdge) {
        setState(UI_GAME);
      }
      break;

    case UI_GAME:
      // Allow UI navigation buttons only while snake is in its own splash state.
      if (snakeGameAllowUiNavigation()) {
        if (rightEdge) {
          setState(UI_SPLASH);
        } else if (leftEdge) {
          setState(UI_INFO);
        }
      }
      break;
  }
}

void setup() {
  pinMode(PIN_LDR, INPUT);
  pinMode(PIN_BTN_LEFT, INPUT_PULLUP);
  pinMode(PIN_BTN_RIGHT, INPUT_PULLUP);
  pinMode(PIN_KILL_SWITCH, INPUT_PULLUP);
  pinMode(PIN_STATUS_LED, OUTPUT);

  Serial.begin(SERIAL_BAUD);
  Wire.begin();

  initSensors();
  readSensors();
  initOled();
  initSnakeGame();
  bootStartMs = millis();
  renderCurrentState(currentState);
}

void loop() {
  const unsigned long now = millis();

  const bool idleRequested = isIdleSwitchActive();
  if (idleRequested) {
    if (currentState != UI_IDLE) {
      setDisplayPowerSave(true);
      snakeGameSetPowerSave(true);
      digitalWrite(PIN_STATUS_LED, LOW);
      setState(UI_IDLE);
    }
    delay(20);
    return;
  }

  if (currentState == UI_IDLE) {
    setDisplayPowerSave(false);
    snakeGameSetPowerSave(false);
    setState(lastActiveState == UI_IDLE ? UI_SPLASH : lastActiveState);
  }

  handleNavigation();

  if (currentState == UI_GAME) {
    snakeGameTick(now);
    if (snakeGameExitRequested()) {
      clearSnakeGameExitRequest();
      setState(UI_SPLASH);
    }
  }

  if (now - lastReadMs >= READ_INTERVAL_MS) {
    readSensors();
    lastReadMs = now;
    if (currentState == UI_INFO) {
      renderInfo();
    }
  }

  if (currentState != UI_GAME) {
    updateDisplayBrightness(lightRaw);
  }
}
